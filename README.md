# MAVLink Companion Telemetry & Failsafe Daemon

Фоновый процесс (daemon) на C++17 для бортового компьютера дрона. Он стоит между полётным контроллером ArduPilot и наземной станцией: принимает телеметрию по MAVLink v2, пересылает её дальше и следит за аварийными ситуациями (failsafe).

Проект учебный: шаг за шагом осваиваем сетевое программирование в Linux, управление памятью и многопоточность в C++ на реальной задаче из мира дронов.

## Архитектура

```
┌──────────────────────┐   UDP / MAVLink v2   ┌──────────────────────┐   UDP / IPC   ┌──────────────────────┐
│  ArduPilot SITL      │ ◄──────────────────► │  C++17 daemon        │ ◄───────────► │  Python GCS          │
│  (Docker)            │                      │  (этот проект)       │               │  (pymavlink)         │
│  полётный контроллер │                      │  бортовой компьютер  │               │  наземная станция    │
└──────────────────────┘                      └──────────────────────┘               └──────────────────────┘
```

| Компонент | Роль на реальном дроне | Чем симулируется |
|---|---|---|
| ArduPilot SITL | Полётный контроллер (Pixhawk) | Docker-контейнер `ardupilot-sitl` |
| C++17 daemon | Бортовой компьютер (Raspberry Pi) | Linux (Ubuntu 24.04, arm64) |
| Python GCS | Наземная станция | Скрипт на `pymavlink` (для визуальной проверки можно подключить QGroundControl) |

## Структура репозитория

```
.
├── CMakeLists.txt        # сборка daemon (C++17, -Wall -Wextra -Wpedantic)
├── src/main.cpp          # исходный код daemon
├── include/mavlink/v2.0  # заголовки MAVLink v2 (C, header-only)
├── test_mavlink.py       # проверка связи с SITL через pymavlink
├── sitl/Dockerfile       # образ ArduPilot SITL нужной версии (собирается под архитектуру хоста)
└── docker-compose.yml    # запуск SITL
```

## Быстрый старт

### 1. Запустить симулятор ArduPilot

```bash
docker compose up -d --build   # первая сборка ~10–20 минут
docker compose logs -f         # ждём строку "Waiting for connection"
```

Версия ArduPilot задаётся в `docker-compose.yml` (`ARDUPILOT_TAG`, по умолчанию `Copter-4.5.7`).

SITL не запускает прошивку, пока кто-то не подключится к порту **5760** (например, QGroundControl или MAVProxy).

| Порт (TCP) | Порт ArduPilot | Назначение |
|---|---|---|
| 5760 | SERIAL0 | Наземная станция |
| 5762 | SERIAL1 | Бортовой компьютер (daemon) |
| 5763 | SERIAL2 | Резерв |

### 2. Проверить связь на Python

```bash
source .venv/bin/activate      # pymavlink уже установлен в .venv
python test_mavlink.py         # ожидаем HEARTBEAT и пять строк ATTITUDE
```

### 3. Собрать и запустить daemon

```bash
cmake -S . -B build
cmake --build build
./build/daemon                 # ожидаем строку "HEARTBEAT sysid=1 ..."
```

## Дорожная карта

**Фаза 1: фундамент и первый сквозной контур (месяц 1)**

- [ ] **Спринт 1 (текущий):** ArduPilot SITL в Docker, проверка канала связи на Python (`pymavlink`), окружение сборки C++17 (CMake, GCC, ASan).
- [ ] **Спринт 2:** низкоуровневая сеть в Linux: Berkeley UDP sockets, неблокирующий ввод-вывод, приём сырых байтов MAVLink на C++.
- [ ] **Спринт 3:** управление памятью в C++17 (RAII, `std::unique_ptr`, `std::vector<uint8_t>`), MAVLink v2 C-заголовки, парсинг `HEARTBEAT`, `ATTITUDE`, `GLOBAL_POSITION_INT`.
- [ ] **Спринт 4:** многопоточность (`std::thread`, `std::mutex`, `std::atomic`), раздельные потоки чтения и отправки, отладка под AddressSanitizer и Valgrind, `systemd`-юнит.

## Текущее состояние

- Daemon подключается к SITL по **TCP 5762**, ждёт первый `HEARTBEAT`, печатает его и завершается. Переход на UDP — в спринте 2.
- Новый образ SITL (`sitl/Dockerfile`) пока не проверен на сборку и запуск. Предыдущий образ `radarku/ardupilot-sitl` (ArduCopter 4.0.3, amd64) падал на хостах arm64 с ошибкой `stack overflow in thread FTP`.
- AddressSanitizer в CMake ещё не подключён.
