#!/usr/bin/env python3
"""Offline, deterministic HTML card extraction; Python 3.9+; no network calls.

For comparison with C++11/C++20: the parsing contract is language-independent,
but this implementation is Python-specific and uses BeautifulSoup 4.
"""
from dataclasses import asdict, dataclass
from urllib.parse import urljoin, urlsplit
import json
import sys
import unittest
from bs4 import BeautifulSoup


@dataclass(frozen=True)
class Article:
    title: str
    url: str
    author: str


def extract(html: str, base_url: str) -> list[Article]:
    """Parse cards without I/O; exclude malformed/foreign links, deduplicate URLs."""
    base = urlsplit(base_url)
    if base.scheme not in ("http", "https") or not base.netloc:
        raise ValueError("base_url must be an absolute HTTP(S) URL")
    soup = BeautifulSoup(html, "html.parser")
    seen = set()
    output = []
    for card in soup.select("article.card"):
        link = card.select_one("a.title[href]")
        if link is None:
            continue
        title = link.get_text(" ", strip=True)
        href = (link.get("href") or "").strip()
        if not href:
            continue
        url = urljoin(base_url, href)
        dest = urlsplit(url)
        if not title or dest.scheme not in ("http", "https") or dest.netloc != base.netloc:
            continue
        if url in seen:
            continue
        author_node = card.select_one(".author")
        author = author_node.get_text(" ", strip=True) if author_node else ""
        output.append(Article(title, url, author or "(unknown)"))
        seen.add(url)
    return output


FIXTURE = """<main>
<article class="card"><a class="title" href="/post/1"> First </a><b class="author"> Ada </b></article>
<article class="card"><a class="title" href="/post/1">Duplicate</a></article>
<article class="card"><a class="title" href="/post/2">Second</a></article>
<article class="card"><span class="author">Missing link</span></article>
<article class="card"><a class="title" href="/blank"> </a></article>
<article class="card"><a class="title" href="javascript:alert(1)">Unsafe</a></article>
<article class="card"><a class="title" href="https://other.example/x">External</a></article>
</main>"""


class ParserTests(unittest.TestCase):
    def test_two_valid_cards(self):
        self.assertEqual([a.title for a in extract(FIXTURE, "https://news.example/")], ["First", "Second"])

    def test_relative_link_and_author(self):
        self.assertEqual(extract(FIXTURE, "https://news.example/")[0], Article("First", "https://news.example/post/1", "Ada"))

    def test_deduplicates(self):
        self.assertEqual(len(extract(FIXTURE, "https://news.example/")), 2)

    def test_missing_author_uses_fallback(self):
        self.assertEqual(extract(FIXTURE, "https://news.example/")[1].author, "(unknown)")

    def test_ignores_incomplete_cards(self):
        self.assertEqual(extract('<article class="card"><a class="title">No href</a></article>', "https://news.example/"), [])

    def test_rejects_foreign_and_javascript_links(self):
        bad = '<article class="card"><a class="title" href="//evil.example/x">Off site</a></article>'
        self.assertEqual(extract(bad, "https://news.example/"), [])
        self.assertEqual(len(extract(FIXTURE, "https://news.example/")), 2)

    def test_empty_href_is_rejected(self):
        html = '<article class="card"><a class="title" href="">Valid title</a></article>'
        self.assertEqual(extract(html, "https://news.example/"), [])

    def test_whitespace_href_is_rejected(self):
        html = '<article class="card"><a class="title" href="   ">Valid title</a></article>'
        self.assertEqual(extract(html, "https://news.example/"), [])

    def test_empty_document(self):
        self.assertEqual(extract("", "https://news.example/"), [])

    def test_invalid_base_url(self):
        with self.assertRaises(ValueError):
            extract(FIXTURE, "relative/path")


if __name__ == "__main__":
    if "--selftest" in sys.argv:
        unittest.main(argv=[sys.argv[0]], verbosity=2)
    else:
        print(json.dumps([asdict(a) for a in extract(FIXTURE, "https://news.example/")], ensure_ascii=False, indent=2))
