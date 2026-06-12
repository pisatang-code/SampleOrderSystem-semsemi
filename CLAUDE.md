# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

---

## 개발 참고 사항

1. **언어**: 모든 소스 코드는 C++17로 작성한다.
2. **한글 인코딩**: 소스 파일은 UTF-8 with BOM으로 저장한다. 콘솔 출력 시 `SetConsoleOutputCP(CP_UTF8)` 및 `SetConsoleCP(CP_UTF8)`을 `main()`에서 초기화한다. 문자열 리터럴에 한글이 포함된 경우 MSVC의 `/utf-8` 컴파일 옵션을 사용한다.
3. **Windows 빌드**: Windows 환경(MSVC, x64)을 기준으로 작성한다. `windows.h` 사용 시 `WIN32_LEAN_AND_MEAN` 및 `NOMINMAX`를 선언하고, POSIX 전용 API는 사용하지 않는다. `#pragma warning(disable:...)` 없이 빌드 경고 없이 컴파일되어야 한다.

---

## 빌드 및 테스트

```bat
# 빌드 (Debug x64)
msbuild SampleOrderSystem22.sln /p:Configuration=Debug /p:Platform=x64

# 빌드 (Release x64)
msbuild SampleOrderSystem22.sln /p:Configuration=Release /p:Platform=x64

# 테스트 실행 (Debug 빌드 완료 시 Post-Build Event로 자동 실행됨)
x64\Debug\SampleOrderSystem22Tests.exe
```

**테스트 프레임워크**: Google Mock (GMock) — NuGet 패키지 `gmock 1.11.0`으로 설치되어 있다.

---

## 문서

- **기능 명세**: `docs/PRD.md` — 모든 기능 구현은 이 문서의 명세를 따른다. 명세에 없는 기능을 임의로 추가하지 않는다.
- **개발 계획**: `docs/PLAN.md` — 아키텍처, Phase별 구현 내용, 설계 결정 사항 정리.

---

## 사람 검토 보고 형식

구현 및 검증이 완료되면 사람이 직접 확인할 수 있도록 아래 형식으로 결과를 정리해서 보고한다.

### 보고 구조

**1. 구현 요약**
- 이번에 구현한 것이 무엇인지 한 문장으로 요약
- 주요 변경 파일 목록

**2. 단위 테스트 결과**
- 전체 테스트 수 및 통과 수 (예: 30/30)
- 새로 추가된 테스트 케이스 목록과 각각이 검증하는 내용

**3. 사람이 확인해야 할 항목**
아래 형식으로 체크리스트를 제공한다.

```
[ ] 확인 항목 — 어떻게 확인하는지 (입력값 / 기대 결과)
```

- 정상 동작 경로 (Happy Path): 주요 기능이 의도대로 동작하는지
- 경계값 / 예외 경로: 잘못된 입력, 엣지 케이스에서 적절히 처리되는지
- 데이터 영속성: 재시작 후에도 데이터가 유지되는지 (해당되는 경우)

**4. 알려진 제약 사항 / 미구현**
- 이번 구현 범위에서 의도적으로 제외한 것
- 다음 Phase에서 다룰 내용

### 보고 시 주의사항
- 기술 용어보다 **동작 관점**으로 설명한다 ("OrderController가 상태를 변경한다" → "주문을 승인하면 상태가 CONFIRMED로 바뀐다")
- 확인 항목은 **실제로 실행해볼 수 있는 구체적인 입력값**을 제시한다
- 테스트를 통과했더라도 **사람이 직접 눈으로 확인해야 할 UI/출력 항목**은 별도로 명시한다

---

## 개발 진행 방식

Phase 순서대로 진행하며, 각 Phase마다 아래 절차를 반드시 따른다.

1. **TDD로 구현** — `/tdd` 스킬을 사용하여 테스트 먼저 작성 → 실패 확인 → 최소 구현 → 통과 확인
2. **Unit Test 검증** — 해당 Phase의 모든 테스트가 통과하면 사람에게 결과 보고 후 git commit
3. **Refactoring** — Clean Code 유지를 위해 리팩토링 수행. 테스트가 계속 통과하는지 확인 후 git commit
4. **CLAUDE.md 진행 상황 업데이트** — Phase 완료 시 아래 "개발 진행 상황" 섹션에 결과를 기록한다
5. **다음 Phase 자동 진행** — 문제가 없으면 사람에게 묻지 않고 즉시 다음 Phase로 넘어간다
6. **문제 발생 시 즉시 중단** — 검증 또는 리팩토링 중 문제가 생기면 사람에게 보고하고 멈춘다

모든 Phase 완료 후:
- **통합 검증** 수행 → git commit
- **최종 Refactoring** 수행 → git commit
- 완료 후 사람에게 보고한다

---

## 개발 진행 상황

| Phase | 상태 | 테스트 | 비고 |
|-------|------|--------|------|
| Phase 1 — 기반 구조 | ✅ 완료 | 30/30 | MVC 구조, JsonStorage, 모델, 메인 메뉴 스켈레톤 |
| Phase 2 — 시료 관리 | ✅ 완료 | 39/39 | SampleController: 등록/조회/검색, 유효성 검증 |
| Phase 3 — 주문 접수 & 승인/거절 | ✅ 완료 | 51/51 | OrderController: 접수/승인/거절, 재고 분기, 주문번호 채번 |
| Phase 4 — 생산라인 | ✅ 완료 | 60/60 | ProductionController: FIFO 큐, 생산 수량 계산, 완료 처리 |
| Phase 5 — 모니터링 & 출고 처리 | ✅ 완료 | 70/70 | MonitoringController: 주문 집계, 재고 현황, 출고 처리 |
| Phase 6 — 테스트 & 더미 데이터 | ✅ 완료 | 75/75 | DummyDataGenerator: 멱등 populate(), 시료 5개/주문 6개 |
| 추가 — 콘솔 UI 연결 | ✅ 완료 | 76/76 | MainController → 전체 서브메뉴 연결, padRight 정렬, PRODUCING 버그 수정 |
