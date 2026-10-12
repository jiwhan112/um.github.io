-- Run once in the Supabase SQL editor. Never put privileged keys in GitHub Pages.
create table if not exists public.atlas_profile (
 user_id integer primary key check (user_id = 1),
 owner_uid uuid not null unique references auth.users(id)
);
create table if not exists public.atlas_state (
 user_id integer not null references public.atlas_profile(user_id),
 storage_key text not null check (length(storage_key) between 1 and 180),
 value text not null,
 updated_at timestamptz not null default now(),
 primary key (user_id, storage_key)
);
alter table public.atlas_profile enable row level security;
alter table public.atlas_state enable row level security;
revoke all on public.atlas_profile from anon;
revoke all on public.atlas_state from anon;
grant select on public.atlas_profile to authenticated;
grant select, insert, update on public.atlas_state to authenticated;
drop policy if exists atlas_owner_profile on public.atlas_profile;
create policy atlas_owner_profile on public.atlas_profile for select to authenticated
 using (user_id = 1 and owner_uid = (select auth.uid()));
drop policy if exists atlas_owner_read on public.atlas_state;
create policy atlas_owner_read on public.atlas_state for select to authenticated
 using (user_id = 1 and exists (select 1 from public.atlas_profile p where p.user_id = atlas_state.user_id and p.owner_uid = (select auth.uid())));
drop policy if exists atlas_owner_insert on public.atlas_state;
create policy atlas_owner_insert on public.atlas_state for insert to authenticated
 with check (user_id = 1 and exists (select 1 from public.atlas_profile p where p.user_id = atlas_state.user_id and p.owner_uid = (select auth.uid())));
drop policy if exists atlas_owner_update on public.atlas_state;
create policy atlas_owner_update on public.atlas_state for update to authenticated
 using (user_id = 1 and exists (select 1 from public.atlas_profile p where p.user_id = atlas_state.user_id and p.owner_uid = (select auth.uid())))
 with check (user_id = 1 and exists (select 1 from public.atlas_profile p where p.user_id = atlas_state.user_id and p.owner_uid = (select auth.uid())));
-- After signing in to GitHub once, inspect auth.users in Supabase SQL editor.
-- Explicitly bind ONLY your verified auth.users.id to the fixed learner number:
-- insert into public.atlas_profile(user_id, owner_uid)
-- values (1, 'YOUR_VERIFIED_AUTH_USERS_UUID');
-- Do not auto-enroll the first visitor: that would let someone claim user #1.
