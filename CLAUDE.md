# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

---

## 개발 참고 사항

1. **언어**: 모든 소스 코드는 C++17로 작성한다.
2. **한글 인코딩**: 소스 파일은 UTF-8 with BOM으로 저장한다. 콘솔 출력 시 `SetConsoleOutputCP(CP_UTF8)` 및 `SetConsoleCP(CP_UTF8)`을 `main()`에서 초기화한다. 문자열 리터럴에 한글이 포함된 경우 MSVC의 `/utf-8` 컴파일 옵션을 사용한다.
3. **Windows 빌드**: Windows 환경(MSVC, x64)을 기준으로 작성한다. `windows.h` 사용 시 `WIN32_LEAN_AND_MEAN`을 선언하고, POSIX 전용 API는 사용하지 않는다. `#pragma warning(disable:...)` 없이 빌드 경고 없이 컴파일되어야 한다.

---

## 빌드 및 테스트

```bat
# 빌드 (Debug x64)
msbuild SampleOrderSystem22.sln /p:Configuration=Debug /p:Platform=x64

# 빌드 (Release x64)
msbuild SampleOrderSystem22.sln /p:Configuration=Release /p:Platform=x64

# 테스트 실행 (빌드 후 생성된 테스트 실행 파일 직접 실행)
x64\Debug\SampleOrderSystem22Tests.exe
```

**테스트 프레임워크**: Google Mock (GMock) — NuGet 패키지 `Microsoft.googletest.v140.windesktop.msvcstl.dyn.rt-dyn`으로 설치되어 있다.

---

## PRD

기능 명세의 기준 문서: `docs/PRD.md`

모든 기능 구현은 PRD의 명세를 따른다. 명세에 없는 기능을 임의로 추가하지 않는다.

---

## 데이터 관리

- 모든 데이터는 JSON 파일로 관리한다. 저장 경로: `db/` 폴더
  - `db/samples.json` — 시료 목록
  - `db/orders.json` — 주문 목록
- 외부 JSON 라이브러리를 사용하지 않는다. DataPersistence PoC 방식대로 직접 파싱/직렬화를 구현한다.
- CRUD 패턴: **Load-Modify-Save** (파일 전체 읽기 → 메모리 수정 → 파일 전체 쓰기)
- `IDataStorage` 인터페이스를 정의하고 `JsonStorage`가 구현한다. 메서드: `Create`, `ReadAll`, `ReadById`, `Update`, `Delete`

---

## 아키텍처: MVC 구조

consoleMVC PoC(`https://github.com/pisatang-code/consoleMVC-semsemi.git`)를 기준으로 구현한다.

```
SampleOrderSystem22/
├── Model/          # 데이터 구조, 비즈니스 로직, JSON 영속성
├── View/           # 콘솔 입출력 (메뉴 렌더링, 입력 수집)
├── Controller/     # Model-View 조율, 메뉴 루프
├── db/             # JSON 데이터 파일
└── main.cpp        # 진입점: 인코딩 초기화, MVC 조립
```

**규칙**
- 인터페이스 클래스: `I` 접두사 (`IModel`, `IView`, `IController`)
- 멤버 변수: `m_` 접두사
- 클래스명: PascalCase / 메서드명: camelCase
- Controller는 Model·View를 `std::shared_ptr`로 주입받는다 (의존성 주입)

---

## 데이터 영속성

DataPersistence PoC(`https://github.com/pisatang-code/DataPersistence-semsemi.git`)를 기준으로 구현한다.

- `IDataStorage<T>` 순수 가상 인터페이스 정의
- `JsonStorage<T>` 구현체: `ParseFile()`, `Serialize()`, `GetValue()`, `EscapeJson()` 포함
- 파일 I/O: `<fstream>`, 컬렉션 조작: `<algorithm>` (`std::remove_if` 등)
- 각 엔티티(Sample, Order)마다 별도 JSON 파일 사용

---

## 모니터링

DataMonitor PoC(`https://github.com/pisatang-code/DataMonitor-semsemi.git`)를 기준으로 구현한다.

- Win32 콘솔 API를 이용한 표 형식 출력
- 재고 상태 표기:

| 상태 | 조건 |
|------|------|
| 여유 | 재고 ≥ 주문 대기 수량 |
| 부족 | 0 < 재고 < 주문 대기 수량 |
| 고갈 | 재고 = 0 |

- 주문 현황은 상태별(`RESERVED` / `PRODUCING` / `CONFIRMED` / `RELEASE`) 건수 및 목록 표시. `REJECTED`는 제외.

---

## 더미 데이터 생성

DummyDataGenerator PoC(`https://github.com/pisatang-code/DummyDataGenerator-semsemi.git`)를 기준으로 구현한다.

- `DataGenerator` 클래스: `std::mt19937` 기반 RNG로 시료명, 수율, 생산시간, 고객명 등 생성
- `genKoreanName()` 패턴으로 한글 고객명 생성
- 생성된 더미 데이터는 `db/samples.json`, `db/orders.json`에 직접 삽입
- 테스트 실행 전 `DummyDataGenerator::populate()`를 호출하여 초기 데이터 세팅

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

## 개발 Phase

### Phase 1 — 기반 구조
- MVC 폴더 구조 및 인터페이스(`IModel`, `IView`, `IController`, `IDataStorage`) 정의
- `Sample`, `Order` 데이터 모델 및 `OrderStatus` 열거형 구현
- `JsonStorage` 구현 (samples.json, orders.json 읽기/쓰기)
- 메인 메뉴 루프 골격 (`ConsoleView` + `MainController`)

### Phase 2 — 시료 관리 (메뉴 1)
- 시료 등록 / 전체 조회(재고 포함) / 이름 검색
- `SampleModel`, `SampleController`, `SampleView`

### Phase 3 — 주문 접수 & 승인/거절 (메뉴 2, 3)
- 주문 접수 → `RESERVED` 상태 생성, 주문번호 채번(`ORD-YYYYMMDD-NNNN`)
- 승인 시 재고 확인 후 자동 분기: `CONFIRMED` 또는 `PRODUCING` + 생산 큐 등록
- 거절 시 `REJECTED` 전환
- `OrderModel`, `OrderController`, `OrderView`

### Phase 4 — 생산라인 (메뉴 5)
- FIFO 생산 큐 (`std::queue`) 관리
- 생산 수량 계산: `ceil(부족분 / (수율 * 0.9))`
- 생산 완료 시 `PRODUCING` → `CONFIRMED` 전환
- 생산 현황 및 대기열 표시

### Phase 5 — 모니터링 & 출고 처리 (메뉴 4, 6)
- 상태별 주문 집계 및 시료별 재고 현황 표시 (여유/부족/고갈)
- `CONFIRMED` 목록 조회 및 출고 실행 → `RELEASE` 전환

### Phase 6 — 테스트 & 더미 데이터
- GMock 기반 단위 테스트 작성 (각 Model, Controller 대상)
- `DummyDataGenerator` 구현 및 테스트용 JSON 데이터 자동 생성

---

## 개발 진행 상황

| Phase | 상태 | 테스트 | 비고 |
|-------|------|--------|------|
| Phase 1 — 기반 구조 | ✅ 완료 | 30/30 | MVC 구조, JsonStorage, 모델, 메인 메뉴 스켈레톤 |
| Phase 2 — 시료 관리 | ✅ 완료 | 39/39 | SampleController: 등록/조회/검색, 유효성 검증 |
| Phase 3 — 주문 접수 & 승인/거절 | ✅ 완료 | 51/51 | OrderController: 접수/승인/거절, 재고 분기, 주문번호 채번 |
| Phase 4 — 생산라인 | ⬜ 대기 | - | |
| Phase 5 — 모니터링 & 출고 처리 | ⬜ 대기 | - | |
| Phase 6 — 테스트 & 더미 데이터 | ⬜ 대기 | - | |
