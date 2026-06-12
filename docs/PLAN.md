# 반도체 시료 생산주문관리 시스템 — 개발 계획서

> 기능 명세 원본: `docs/PRD.md`

---

## 1. 프로젝트 개요

| 항목 | 내용 |
|------|------|
| 언어 | C++17 (MSVC v143, x64) |
| 빌드 | Visual Studio 2022 / MSBuild |
| 테스트 | Google Mock (GMock) — NuGet `gmock 1.11.0` |
| 데이터 | JSON 파일 (`db/samples.json`, `db/orders.json`) |
| 개발 방식 | TDD (Red → Green → Refactor) |

---

## 2. 아키텍처

### 2-1. 레이어 구조

```
main.cpp
  └── MainController          ← 전체 메뉴 루프 및 서브메뉴 라우팅
        ├── SampleController  ← 시료 비즈니스 로직
        ├── OrderController   ← 주문 비즈니스 로직
        ├── ProductionController ← 생산라인 로직
        ├── MonitoringController ← 모니터링/출고 로직
        └── ConsoleView       ← 콘솔 입출력

Model Layer (데이터 및 영속성)
  ├── Sample / Order / OrderStatus
  ├── IDataStorage<T>         ← 순수 가상 인터페이스
  ├── SampleStorage           ← JSON CRUD (samples.json)
  ├── OrderStorage            ← JSON CRUD (orders.json)
  ├── JsonUtil                ← 공용 JSON 파서/직렬화
  └── DummyDataGenerator      ← 테스트용 더미 데이터 주입
```

### 2-2. 폴더 구조

```
SampleOrderSystem22/
├── Model/          # 데이터 구조, JSON 영속성
├── View/           # 콘솔 입출력
├── Controller/     # 비즈니스 로직 및 메뉴 라우팅
├── db/             # JSON 데이터 파일 (런타임 생성)
└── main.cpp

SampleOrderSystem22Tests/
├── ModelTests.cpp
├── StorageTests.cpp
├── SampleManagementTests.cpp
├── OrderManagementTests.cpp
├── ProductionLineTests.cpp
├── MonitoringTests.cpp
├── DummyDataTests.cpp
└── TestMain.cpp
```

### 2-3. 주문 상태 전이

```
RESERVED → (승인, 재고 충분) → CONFIRMED → RELEASE
RESERVED → (승인, 재고 부족) → PRODUCING → CONFIRMED → RELEASE
RESERVED → (거절)            → REJECTED
```

- `REJECTED`는 모니터링 집계에서 제외
- 재고 상태 판정: 여유(stock ≥ pending) / 부족(0 < stock < pending) / 고갈(stock = 0)

### 2-4. 핵심 공식

```
부족분       = max(0, 주문량 - 현재재고)
실 생산량    = ceil(부족분 / (수율 × 0.9))
총 생산 시간 = 평균생산시간 × 실생산량
```

---

## 3. Phase별 구현 계획

### Phase 1 — 기반 구조

**목표:** 전체 골격 구축. 이후 모든 Phase가 이 기반 위에서 동작.

| 항목 | 내용 |
|------|------|
| 상태 | ✅ 완료 |
| 테스트 | 30개 |

**구현 내용:**
- `OrderStatus` 열거형 및 문자열 변환 함수
- `Sample`, `Order` 데이터 구조 (toJsonObject / fromJsonObject)
- `IDataStorage<T>` 순수 가상 인터페이스 (Create / ReadAll / ReadById / Update / Delete)
- `JsonUtil`: 외부 라이브러리 없이 직접 구현한 JSON 파서 및 직렬화
  - `parseObjects()`: `[{...},{...}]` → 객체 문자열 벡터
  - `serializeArray<T>()`: 템플릿 직렬화 헬퍼
  - `escapeJson()`, `getValue()`, `readFile()`, `writeFile()`
- `SampleStorage`, `OrderStorage`: Load-Modify-Save CRUD
- `ConsoleView` (스켈레톤): 메인 메뉴 렌더링, 입력 수집
- `MainController` (스켈레톤): 메뉴 루프, 서브메뉴 스텁

**설계 결정:**
- 외부 JSON 라이브러리 미사용 (DataPersistence PoC 패턴 준수)
- 파일 경로를 생성자 인자로 주입 → 테스트 시 임시 파일 사용 가능
- `serializeArray<T>()` 템플릿으로 SampleStorage / OrderStorage 중복 제거

---

### Phase 2 — 시료 관리 (메뉴 1)

**목표:** 시료 등록 / 목록 조회 / 이름 검색

| 항목 | 내용 |
|------|------|
| 상태 | ✅ 완료 |
| 테스트 | 39개 (Phase1 30 + 신규 9) |

**구현 내용:**
- `SampleController::registerSample()`: 중복 ID / 수율 범위 / 생산시간 유효성 검증
- `SampleController::getAllSamples()`: 전체 목록 반환
- `SampleController::searchByName()`: 대소문자 무관 부분 일치 검색

**설계 결정:**
- `toLower()` 헬퍼를 무명 네임스페이스에 분리하여 재사용

---

### Phase 3 — 주문 접수 & 승인/거절 (메뉴 2, 3)

**목표:** 주문 생성, 상태 전이, 재고 차감 자동화

| 항목 | 내용 |
|------|------|
| 상태 | ✅ 완료 |
| 테스트 | 51개 (Phase2 39 + 신규 12) |

**구현 내용:**
- `OrderController::receiveOrder()`: 주문번호 채번 (`ORD-YYYYMMDD-NNNN`), RESERVED 생성
- `OrderController::approveOrder()`: 재고 충분 → CONFIRMED + 재고 차감 / 부족 → PRODUCING
- `OrderController::rejectOrder()`: REJECTED 전환
- `OrderController::getReservedOrders()`: RESERVED 목록 조회

**설계 결정:**
- 주문번호 시퀀스: 당일 기존 주문 중 최대 시퀀스 + 1 (재시작 시에도 연속성 유지)
- `getReservedOrder()` 헬퍼로 조회+검증 로직 중복 제거
- `NOMINMAX` 정의로 `windows.h`의 `max` 매크로 충돌 방지

---

### Phase 4 — 생산라인 (메뉴 5)

**목표:** FIFO 생산 큐 관리, 생산 수량 계산, 생산 완료 처리

| 항목 | 내용 |
|------|------|
| 상태 | ✅ 완료 |
| 테스트 | 60개 (Phase3 51 + 신규 9) |

**구현 내용:**
- `ProductionController::calcActualProduction()`: `ceil(부족분 / (수율 × 0.9))`
- `ProductionController::getQueue()`: PRODUCING 주문 목록 (삽입 순서 = FIFO)
- `ProductionController::completeProduction()`: 생산 완료 → 재고 추가 → 주문분 차감 → CONFIRMED

**설계 결정:**
- `calcActualProduction()`을 static 메서드로 분리 → 표시 로직에서도 재사용 가능
- FIFO는 `OrderStorage` 삽입 순서 보장으로 별도 큐 자료구조 불필요
- **버그 수정 이력:** 초기 구현에서 `order.quantity`를 부족분으로 잘못 사용 → `max(0, order.quantity - sample.stock)`으로 수정

---

### Phase 5 — 모니터링 & 출고 처리 (메뉴 4, 6)

**목표:** 상태별 주문 집계, 재고 현황 모니터링, 출고 처리

| 항목 | 내용 |
|------|------|
| 상태 | ✅ 완료 |
| 테스트 | 70개 (Phase4 60 + 신규 10) |

**구현 내용:**
- `MonitoringController::getOrderCountByStatus()`: 상태별 건수 집계 (REJECTED 제외)
- `MonitoringController::getOrdersByStatus()`: 상태별 주문 목록 (REJECTED 요청 시 빈 목록)
- `MonitoringController::getStockStatuses()`: 시료별 재고 상태 (여유/부족/고갈)
- `MonitoringController::shipOrder()`: CONFIRMED → RELEASE
- `MonitoringController::getConfirmedOrders()`: 출고 대기 목록

**설계 결정:**
- `getStockStatuses()`: 주문 목록 선순회로 `unordered_map` 집계 후 시료 목록 순회 → O(n) 구현
- 재고 상태 판정은 REJECTED 제외한 유효 주문만 대상

---

### Phase 6 — 테스트 & 더미 데이터

**목표:** 테스트 자동화 환경 완성, 더미 데이터로 초기 DB 구성

| 항목 | 내용 |
|------|------|
| 상태 | ✅ 완료 |
| 테스트 | 75개 (Phase5 70 + 신규 5) |

**구현 내용:**
- `DummyDataGenerator::populate()`: 멱등성 보장 (기존 데이터 있으면 건너뜀)
  - 시료 5개: AlphaSample(0.90) / BetaSample(0.85) / GammaSample(0.95) / DeltaSample(0.80) / EpsilonSample(0.92)
  - 주문 6개: CONFIRMED / PRODUCING / RESERVED / RELEASE / REJECTED / RESERVED 각 1건
- Debug 빌드 시 Post-Build Event로 테스트 자동 실행

**설계 결정:**
- `populate()`가 한 번만 실행되도록 기존 데이터 유무로 멱등성 보장
- 테스트는 모두 임시 파일(`*_tmp.json`) 사용, SetUp/TearDown에서 삭제

---

### 추가 — 생산라인 실시간 타이머

**목표:** 생산 승인 시각 기록 후 경과 시간 기반 진행률/자동 완료 구현

| 항목 | 내용 |
|------|------|
| 상태 | ✅ 완료 |
| 테스트 | 83개 (신규 7) |

**구현 내용:**
- `Order::approvedAt`: PRODUCING 전환 시 `time(nullptr)` 값을 epoch 초 문자열로 저장
- `OrderController::approveOrder()`: PRODUCING 분기 시 `approvedAt` 기록
- `ProductionController::ProductionInfo` 구조체: 진행률/경과시간/완료예정시각 포함
- `ProductionController::getQueueInfo()`: 실시간 경과 시간 계산 → 진행률(%) / 완료예정(HH:MM)
- `ProductionController::calcTotalTimeMinutes()`: 총 생산 시간 = 실생산량 × avgProductionTime
- `ProductionController::autoComplete()`: 총 생산 시간 초과 주문 자동 CONFIRMED 전환
  + `approvedAt` 없는 기존 PRODUCING 주문 마이그레이션 (앱 실행 시 현재 시각 기록)
- `MainController::buildSummary()`: 메인 메뉴 진입마다 `autoComplete()` 호출
- `handleProductionLine()`: Enter 새로고침, 진행률 표(진행률% / 경과/총시간 / 완료예정) 표시

**설계 결정:**
- `time(nullptr)` 사용 (POSIX, Windows 모두 지원) — epoch 초 단위 저장으로 계산 단순화
- `approvedAt` 없는 기존 주문은 자동 완료 금지 (하위 호환) — 단, 앱 실행 시 마이그레이션으로 현재 시각 자동 기록
- `getQueueInfo()`는 순수 조회(읽기만), 마이그레이션은 `autoComplete()`에서만 수행

**신규 테스트:**
| 테스트 | 검증 내용 |
|--------|-----------|
| `CalcTotalTimeMinutes` | 총 생산 시간 공식 |
| `JsonRoundTripWithApprovedAt` | approvedAt 직렬화/역직렬화 |
| `BackwardCompatEmptyApprovedAt` | 기존 JSON 하위 호환 |
| `AutoCompleteFinishesExpiredOrder` | 과거 시각 → 자동 완료 |
| `AutoCompleteSkipsNonExpiredOrder` | 미래 시각 → 자동 완료 안 됨 |
| `AutoCompleteSkipsOrderWithoutApprovedAt` | approvedAt 없으면 자동 완료 안 됨 |
| `GetQueueInfoPopulatesFields` | ProductionInfo 구조체 정상 채워짐 |

---

### 추가 — 콘솔 UI 연결 및 버그 수정

**목표:** Phase 1~6에서 구현된 비즈니스 로직을 실제 콘솔 화면에 연결

| 항목 | 내용 |
|------|------|
| 상태 | ✅ 완료 |
| 테스트 | 76개 (신규 1: CompleteProductionWithExistingStock) |

**구현 내용:**
- `ConsoleView` 확장: `getString()`, `getInt()`, `getDouble()`, `pressEnterToContinue()`, `showHeader()`, `showSeparator()`
- `ConsoleView::displayWidth()` / `padRight()`: 한글(2칸) 포함 표 정렬 헬퍼
- `MainController` 재구성: 6개 서브메뉴 핸들러 전체 구현
  - `handleSampleManagement()`: 등록 / 조회 / 검색
  - `handleOrderReception()`: 주문 접수
  - `handleOrderApproval()`: 주문 목록 → 번호 선택 → 승인/거절
  - `handleMonitoring()`: 주문 현황 + 재고 현황
  - `handleProductionLine()`: 생산 큐 조회 + 완료 처리
  - `handleShipment()`: 출고 대기 목록 → 번호 선택 → 출고

**수정된 버그:**
| 버그 | 원인 | 수정 |
|------|------|------|
| 한글 오타 다수 | hex 인코딩 수동 작성 오류 | 디코더 스크립트로 전수 검사 후 수정 |
| 표 열 정렬 불일치 | `setw()`가 바이트 수 기준 (한글 3바이트 = 2칸) | `displayWidth()` / `padRight()` 도입 |
| PRODUCING 과잉 생산 | `order.quantity` 전체를 부족분으로 사용 | `max(0, qty - stock)`으로 실제 부족분 계산 |

---

## 4. 개발 중 주요 결정 사항

| 결정 | 이유 |
|------|------|
| 외부 JSON 라이브러리 미사용 | DataPersistence PoC 방향 준수, 의존성 최소화 |
| Load-Modify-Save 패턴 | 단순성 우선, 동시성 요구사항 없음 |
| `calcActualProduction()` static 분리 | 표시 계산과 실제 처리 로직에서 동일 공식 재사용 |
| 테스트용 파일 경로 주입 | `SampleStorage("test_tmp.json")` 형태로 테스트 격리 |
| `displayWidth()` 직접 구현 | Windows 콘솔에서 한글 2칸 처리, 외부 라이브러리 불필요 |
| FIFO = 삽입 순서 | `OrderStorage`가 삽입 순서를 유지하므로 별도 큐 자료구조 불필요 |
| `approvedAt` epoch 초 문자열 | 계산이 단순 (뺄셈으로 경과 초 산출), JSON 직렬화 용이 |
| 마이그레이션을 `autoComplete()` 에서 처리 | 앱 시작 시 자동 실행되어 기존 데이터에 투명하게 적용 |

---

## 5. 향후 개선 가능 사항

- 주문번호 시퀀스를 영속적으로 관리 (재시작 후 중복 방지 강화)
- 시료별 재고 초기화 기능 (관리자 메뉴)
- 단일 JSON 파일을 DB로 사용 중이므로 데이터 증가 시 성능 검토 필요
- 생산라인 FIFO 엄밀화: 선행 주문 완료 후 다음 주문 타이머 시작 (현재는 각 주문이 독립 타이머)
