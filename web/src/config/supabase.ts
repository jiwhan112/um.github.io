/** Public application connection only. Never put service_role, OAuth secrets or user tokens here. */
export type CloudConfig = { url: string; key: string };
export const siteSupabase: Readonly<CloudConfig> = Object.freeze({
  url: 'https://hmnnojgsoicoyzdfidcq.supabase.co',
  key: 'sb_publishable_NoVF2fWi7w5afJRJUEHJVA_garo6iMA',
});

/** Retain valid legacy browser settings without trusting malformed or privileged credentials. */
export function parseCloudConfig(value: unknown): CloudConfig | null {
  if (!value || typeof value !== 'object' || Array.isArray(value)) return null;
  const c = value as Record<string, unknown>;
  if (typeof c.url !== 'string' || typeof c.key !== 'string') return null;
  const url = c.url.trim().replace(/\/+$/, '');
  const key = c.key.trim();
  if (!/^https:\/\/[a-z0-9-]+\.supabase\.co$/.test(url)) return null;
  if (/^sb_publishable_[A-Za-z0-9_-]+$/.test(key)) return { url, key };
  // Legacy anon JWTs are public API keys too. A service_role JWT must never be accepted.
  try {
    const parts = key.split('.');
    if (parts.length !== 3) return null;
    const encoded = parts[1].replace(/-/g, '+').replace(/_/g, '/');
    const payload = JSON.parse(atob(encoded.padEnd(Math.ceil(encoded.length / 4) * 4, '='))) as {role?: string};
    return payload.role === 'anon' ? { url, key } : null;
  } catch { return null; }
}
