# ATLAS NOTE · 기기 간 학습 상태 동기화

현재 `master`의 GitHub Pages는 읽기 전용 정적 호스팅이므로 계정 번호를 `1`로 정하는 것만으로는 기기 간 저장이 되지 않습니다. React는 **기존 localStorage를 지우지 않고** 필요할 때 Supabase REST API와 동기화합니다. 클라우드 설정이 없는 동안에도 모든 체크·메모는 로컬에서 계속 작동합니다.

## 저장 항목 점검

| 기능 | 기존 저장 키 | 적용 |
| --- | --- | --- |
| 게임·AI 브리핑의 읽음/북마크 | `jiwhan-briefing-v1` | 동기화 |
| 언어 학습 섹션 체크 | `atlas-language-sections-v1` | 동기화, 목차 완료 배지 |
| 언어 복습 완료 | `atlas-review-v1` | 동기화, 오늘/밀림/완료 분리 |
| 사고력·오픈소스·실험 문서의 학습 완료 | `atlas-reading-v1` | 동기화 |
| 사고력 C++ 메모 | `thinking-ctci-...-draft` | 동기화 |
| 일반 문서의 코드 메모 | `atlas-code-...` | 동기화 |
| 검색, 탭, 화면 필터 | URL·컴포넌트 상태 | 서버 저장 불필요 |

개인 학습 기록은 `language-study/data/progress.json`에 **저장하지 않습니다**. 그 파일과 `catalog.json`은 기존 GitHub Actions의 자동 생성·복습 일정 용도로 계속 남겨둡니다.

## 1. Supabase 프로젝트 준비 (최초 1회, 사용자 작업)

1. 개인 Supabase 프로젝트 생성.
2. SQL 편집기에서 [`web/supabase/schema.sql`](./supabase/schema.sql) 실행. `atlas_profile`의 소유자는 **자동 등록하지 않습니다**.
3. Supabase Authentication → Providers → GitHub를 활성화. GitHub OAuth App을 만들고 callback URL을 Supabase 화면에 나온 `https://<project-ref>.supabase.co/auth/v1/callback`으로 설정.
4. Supabase Authentication → URL Configuration → redirect allowlist에 `https://jiwhan112.github.io/um.github.io/` 추가. 앱 메인 경로에서 OAuth 해시 토큰을 수신합니다.
5. 사이트 상단 **기록 동기화**에서 project URL과 **publishable/anon key만** 입력하고 설정을 저장. **service_role/secret key 및 GitHub OAuth client secret은 절대로 사이트에 넣지 않습니다.** 이 정보는 GitHub Pages 커밋에도 포함되지 않습니다.
6. 같은 메뉴의 **GitHub 로그인**으로 본인 계정에서 1회 로그인.
7. Supabase SQL 편집기에서 `auth.users`의 GitHub 사용자 인증을 **직접 확인** 후 해당 사용자의 `id`(UUID)를 다음과 같이 정확히 하나만 등록:
   ```sql
   insert into public.atlas_profile(user_id,owner_uid)
   values (1,'YOUR_VERIFIED_AUTH_USERS_UUID');
   ```
8. 사이트에서 **지금 동기화**. 상태가 `동기화됨`으로 바뀌고 `atlas_state`에 실제 행이 생기는지 확인.
9. 다른 브라우저/기기에서도 위 5~6을 실행하면 사용자 번호 1의 같은 데이터를 읽고 기록합니다.

Supabase는 현재 이 대화에서 사용자 소유 계정으로 생성하거나 GitHub OAuth 비밀 키를 등록할 수 없습니다. 따라서 **이 절차를 완료하기 전에는 기기 간 동기화가 실제로 활성화된 것이 아닙니다.** 

## 2. 오프라인·기존 기록 처리

- 동기화 대상 키는 React의 `useStored`에서 변경을 감지하고, 온라인 복귀·창 포커스·수동 요청 시 재시도합니다. 서버 장애 시 로컬 학습 및 체크 사용은 유지합니다.
- 다른 기기에서 이미 저장된 클라우드 값과 로컬 값이 충돌하면 서버 값을 우선 적용하고 이전 로컬 내용은 `atlas-cloud-conflicts-v1`에 백업합니다. 같은 데이터에 두 기기가 동시에 편집할 경우 마지막 쓰기가 우선하므로, 중요한 코드 메모는 화면의 **로컬 백업** 버튼으로 추가 보관하세요.
- `atlas-cloud-settings-v1`, `atlas-cloud-session-v1`, `atlas-cloud-pending-v1`, `atlas-cloud-conflicts-v1`은 동기화 대상 데이터가 아닌 연결 상태·충돌 보호용 로컬 키입니다.
- 서버에는 학습 상태와 메모만 기록됩니다. 기사 본문이나 Lesson JSON, 검색어, UI 필터 값은 업로드하지 않습니다.
- 사용자 1번 테이블의 RLS는 인증된 **특정 auth.users UUID 한 명만** 읽고 쓸 수 있도록 제한합니다. 사용자 번호 1만으로 쓰기를 허용하지 않습니다.

## 검증

```sh
cd web
npm ci
npm run check
```

Cloud REST 실동기화는 별도 프로젝트·계정이 설정돼야 실제 확인할 수 있습니다. CI는 설정 없이 기존 브라우저 저장 호환성·UI·빌드를 검사합니다.
