# ATLAS NOTE · 기기 간 학습 기록 동기화

## 사용자: 새 기기에서도 GitHub 로그인만 하기

사이트의 기본 연결은 `web/src/config/supabase.ts`가 관리합니다. 프로젝트 URL은 `https://hmnnojgsoicoyzdfidcq.supabase.co`입니다. 브라우저에서 사용할 수 있는 **publishable key만** 이 공통 설정에 포함되어 있습니다. 사용자 비밀번호, GitHub OAuth Client Secret, Supabase secret/service_role 키, 로그인 토큰은 포함하지 않습니다.

1. 기존 기록이 있는 브라우저에서 **기록 동기화 → 로컬 백업**으로 JSON 사본을 보관합니다.
2. **GitHub 로그인**을 누릅니다. 프로젝트 주소나 공개 키를 입력할 필요가 없습니다.
3. 서버의 학습자 1번 소유자 설정이 완료되었다면 로그인 후 자동 동기화됩니다. **지금 동기화**로 재시도할 수 있습니다.
4. 다른 브라우저와 모바일에서도 사이트를 열고 **GitHub 로그인**만 진행합니다. 로그인 세션 자체는 기기 간 공유하지 않습니다.
5. 두 기기에서 북마크·체크·풀이 메모가 실제로 전달되는지 확인합니다. UI/배포 검사 통과와 실제 계정의 양방향 동기화 검증은 구분합니다.

초기 설정 없이도 체크와 메모는 로컬에 계속 보관됩니다. 인터넷이나 서버 연결이 끊겨도 로컬 기록은 지우지 않습니다.

## 기존 브라우저 설정과 로그인 보존

- 연결 설정이 없는 새 브라우저: 공통 프로젝트 주소와 publishable key를 자동 사용합니다.
- 같은 프로젝트의 이전 연결 설정: 공통 공개 키를 사용하되 기존 로그인 세션·학습 기록·메모·전송 대기 정보는 초기화하지 않습니다. 주소 끝의 `/` 차이는 정규화합니다.
- 다른 프로젝트의 유효한 설정: 그 프로젝트의 토큰/데이터를 기본 서버에 잘못 보내지 않도록 기존 연결을 유지합니다. 이 경우에만 **ATLAS NOTE 기본 서버로 전환** 버튼이 나타납니다. 명시적으로 전환하면 그 브라우저의 이전 로그인 세션만 제거하고 다시 로그인하도록 합니다. 로컬 학습 기록은 유지합니다.
- 공개 키 교체는 프로젝트 자체가 바뀌지 않는 한 로그인 세션을 삭제하는 이유가 아닙니다. 만료되거나 취소된 실제 사용자 세션은 다시 로그인해야 할 수 있습니다.
- 연결값을 변경할 때는 `src/config/supabase.ts`에서 한 번 수정하고 정상 빌드·배포합니다. 컴파일된 `assets/react/` 파일을 직접 편집하지 않습니다.

## 동기화되는 기록

| 기능 | 기존 저장 키 |
| --- | --- |
| 브리핑 읽음·북마크 | `jiwhan-briefing-v1` |
| 언어 레슨 섹션 체크 | `atlas-language-sections-v1` |
| 복습 완료 | `atlas-review-v1` |
| 사고력·오픈소스·실험 문서 완료 | `atlas-reading-v1` |
| 사고력 C++ 풀이 메모 | `thinking-ctci-...-draft` |
| 일반 문서 코드 메모 | `atlas-code-...` |

검색·탭·화면 필터, 기사·레슨 본문과 접속 설정/인증 토큰은 학습 상태 테이블에 업로드하지 않습니다. `language-study/data/progress.json`과 카탈로그는 예약 발행·복습 일정 자료이며 개인 학습 기록 저장소가 아닙니다.

## 관리자: 최초 한 번 필요한 서버 설정

기본 연결값을 배포해도 테이블·GitHub 인증·소유자 권한이 저절로 생성되는 것은 아닙니다.

1. 대상 Supabase 프로젝트 SQL 편집기에서 [`supabase/schema.sql`](./supabase/schema.sql)을 검토 후 실행합니다. 기존 데이터를 지우거나 방문자를 자동으로 소유자로 등록하지 않습니다.
2. Supabase Authentication → Sign In / Providers → GitHub를 활성화하고 GitHub OAuth App의 Client ID/Client Secret을 설정합니다. 비밀 값은 Supabase 관리 화면에만 입력합니다.
3. GitHub OAuth App의 Authorization callback URL: `https://hmnnojgsoicoyzdfidcq.supabase.co/auth/v1/callback`.
4. Supabase Site URL 및 Redirect URL: `https://jiwhan112.github.io/um.github.io/`.
5. 사이트에서 본인 GitHub 계정으로 로그인한 후, Supabase의 `auth.users`에서 그 사용자임을 직접 확인합니다.
6. 검증한 UUID 하나만 `atlas_profile`의 학습자 1번에 연결합니다:
   ```sql
   insert into public.atlas_profile(user_id, owner_uid)
   values (1, 'YOUR_VERIFIED_AUTH_USERS_UUID');
   ```
   이미 소유자가 등록되어 있으면 덮어쓰지 말고 기존 매핑부터 확인합니다.
7. 로그인하지 않은 요청과 다른 사용자의 요청이 상태를 읽거나 쓸 수 없는지 확인하고, 본인 계정의 읽기·쓰기와 실제 두 기기 동기화를 검증합니다.

공개 키는 앱을 식별할 뿐 사용자 권한을 부여하지 않습니다. `atlas_profile`/`atlas_state`의 RLS를 끄거나 사용자 번호 1만으로 쓰기를 허용하지 마세요. service_role, `sb_secret_...`, OAuth secret과 사용자 토큰을 소스/빌드/로그에 포함하지 않습니다.

## 저장·오프라인·충돌의 현재 범위

기존 `useStored`가 지원 키 변경을 감지하며 온라인 복귀·창 포커스·수동 동기화 때 재시도합니다. 현재 구현은 저장 키 단위의 클라우드 미러입니다. 미전송 로컬 변경이 없는 키는 서버 값을 가져오며 기존 로컬 값은 `atlas-cloud-conflicts-v1`에 백업합니다. 미전송 변경은 서버로 올립니다. 여러 기기가 같은 저장 키를 동시에 바꾸면 마지막 쓰기가 우선하므로, 중요한 메모는 로컬 백업도 보관하세요. 이번 기본 연결 변경은 충돌 병합 알고리즘을 재구현하지 않습니다.

`atlas-cloud-settings-v1`, `atlas-cloud-session-v1`, `atlas-cloud-pending-v1`, `atlas-cloud-conflicts-v1`은 로컬 연결·세션·대기·복구 정보입니다. 임의로 초기화하지 않습니다.

## 개발 검증

```sh
cd web
npm ci
npm run check
```

`tests/cloud-defaults.spec.ts`는 데스크톱/모바일 새 브라우저, 기본 로그인 URL, 기존 세션·기록 보존, 신규 OAuth 콜백, 세션 갱신, 소유자 미등록 차단, 다른 프로젝트 연결 보존, 잘못된 설정을 테스트합니다. 외부 인증/REST는 가짜 토큰과 테스트 응답으로 대체하며 실제 개인 기록을 수정하지 않습니다. 실제 Supabase 로그인 및 본인 소유자 등록 성공 여부는 별도의 실사용 검증으로 확인해야 합니다.
