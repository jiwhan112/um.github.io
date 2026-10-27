/** Bound manual continuation to the user's approved, verified source-video inventory. */
export function validateGameDesignPlan(plan, episodes, feed) {
  const fail = message => { throw new Error('Game design plan: ' + message); };
  const categories = ['Game Concepts', 'Game Essence', 'Planning & Game Design', 'Design Specifics', 'UI'];
  if (plan?.version !== 1 || plan.mode !== 'manual_continuation' || plan.approvedDate !== '2026-10-27') fail('unrecognized approval');
  if (plan.channelId !== 'UCv1DvRY5PyHHt3KN9ghunuw' || plan.entries?.length !== 104) fail('verified 104-video inventory required');
  if (feed.targetEpisodes !== 104 || feed.status !== 'completed') fail('bound or paused/completed schedule changed');
  const ids = new Set();
  const counts = Object.fromEntries(categories.map(category => [category, 0]));
  for (const [index, entry] of plan.entries.entries()) {
    if (entry.number !== index + 1 || !/^[A-Za-z0-9_-]{11}$/.test(entry.videoId) || ids.has(entry.videoId)) fail('duplicate or nonsequential manifest entry');
    if (!categories.includes(entry.sourceCategory) || !/^\d{4}-\d{2}-\d{2}$/.test(entry.sourceDate) || !entry.title?.trim()) fail('invalid source metadata');
    if (entry.sourceUrl !== 'https://www.youtube.com/watch?v=' + entry.videoId || !Number.isFinite(Date.parse(entry.publishedTimestamp)) || entry.sourceDate !== entry.publishedTimestamp.slice(0, 10)) fail('source URL or publication timestamp mismatch');
    if (index && Date.parse(entry.publishedTimestamp) < Date.parse(plan.entries[index - 1].publishedTimestamp)) fail('publication timestamps out of order');
    if (index && entry.sourceDate < plan.entries[index - 1].sourceDate) fail('source dates out of order');
    ids.add(entry.videoId);
    counts[entry.sourceCategory] += 1;
  }
  for (const [category, count] of Object.entries({'Game Concepts':12,'Game Essence':29,'Planning & Game Design':30,'Design Specifics':18,'UI':15})) if (counts[category] !== count) fail('source category count drift');
  if (!Array.isArray(episodes) || episodes.length < 12 || episodes.length > plan.entries.length) fail('invalid published prefix length');
  for (const [index, episode] of episodes.entries()) {
    const expected = plan.entries[index];
    if (episode.video_id !== expected.videoId || Number(episode.number) !== expected.number) fail('published catalog is not the approved chronological prefix');
    if (index < 12 && (episode.path !== expected.existingPath || episode.date !== expected.existingDate)) fail('existing lesson identity or date changed');
  }
}
