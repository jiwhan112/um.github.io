#!/usr/bin/env python3
"""Reconcile language-study indexes without re-writing lesson contents."""
import json
from datetime import date, timedelta
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "language-study" / "data"
CAT = ROOT / "catalog.json"
PROG = ROOT / "progress.json"
INTERVALS = (1, 3, 7, 14, 30)

def read(path):
    return json.loads(path.read_text(encoding="utf-8"))

def write_if_changed(path, data):
    content = json.dumps(data, ensure_ascii=False, indent=2) + "\n"
    if not path.exists() or path.read_text(encoding="utf-8") != content:
        path.write_text(content, encoding="utf-8")
        print("updated:", path)

def unique(items):
    out, seen = [], set()
    for x in items:
        key = (x.get("language"), x.get("from_lesson"), x.get("term"), x.get("due_date"))
        if key not in seen:
            seen.add(key)
            out.append(x)
    return out

catalog, progress = read(CAT), read(PROG)
today = date.today().isoformat()
new_lessons = []
for language in ("english", "japanese"):
    folder = ROOT / language
    found = []
    for path in sorted(folder.glob("*.json")):
        lesson = read(path)
        num, day = lesson.get("id"), lesson.get("date")
        if not isinstance(num, int) or not isinstance(day, str):
            raise ValueError(f"invalid lesson id/date: {path}")
        if day > today:
            raise ValueError(f"future-dated lesson: {path}")
        found.append((num, day, lesson, path))
    found.sort()
    for expected, (num, day, _, path) in enumerate(found, 1):
        if num != expected or path.stem != f"{num:03d}":
            raise ValueError(f"nonsequential lesson file: {path}")
    if len({day for _, day, _, _ in found}) != len(found):
        raise ValueError(f"duplicate lesson dates: {language}")
    before = {x["id"] for x in catalog.get(language, [])}
    catalog[language] = [
        {"id": num, "date": day, "title": item["title"], "file": f"data/{language}/{path.name}"}
        for num, day, item, path in found
    ]
    progress[language]["latest_lesson"] = found[-1][0] if found else 0
    new_lessons.extend((language, num, day, lesson) for num, day, lesson, _ in found if num not in before)

review = progress.setdefault("review", {})
review["schedule_days"] = list(INTERVALS)
due = list(review.get("due", []))
upcoming = list(review.get("upcoming", []))
for language, num, day, lesson in new_lessons:
    terms = (
        [x["text"] for x in lesson.get("expressions", []) if x.get("text")]
        if language == "english" else
        [x["jp"] for x in lesson.get("kana", []) + lesson.get("words", []) if x.get("jp")]
    )
    for term in dict.fromkeys(terms):
        for offset in INTERVALS:
            when = (date.fromisoformat(day) + timedelta(days=offset)).isoformat()
            record = {"language": language, "from_lesson": num, "term": term,
                      "due_date": when, "status": "due" if when <= today else "scheduled"}
            (due if when <= today else upcoming).append(record)
remaining = []
for entry in upcoming:
    if entry.get("due_date", "9999") <= today:
        due.append({**entry, "status": "due"})
    else:
        remaining.append(entry)
review["due"] = unique(due)
review["upcoming"] = unique(remaining)
if new_lessons:
    progress["updated"] = today
write_if_changed(CAT, catalog)
write_if_changed(PROG, progress)
print(f"Indexed English {progress['english']['latest_lesson']}, Japanese {progress['japanese']['latest_lesson']}; due={len(review['due'])}")
