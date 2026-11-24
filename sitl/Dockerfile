# Виртуальный дрон: ArduPilot SITL заданной версии, собирается под архитектуру хоста (arm64 на Apple Silicon)

# 1. Базовый образ — чистая Ubuntu 22.04
FROM ubuntu:22.04

# 2. Версия ArduPilot. Можно переопределить при сборке: --build-arg ARDUPILOT_TAG=...
ARG ARDUPILOT_TAG=Copter-4.5.7
ENV DEBIAN_FRONTEND=noninteractive

# 3. Системные зависимости для сборки
RUN apt-get update && apt-get install -y --no-install-recommends \
        git ca-certificates build-essential python3 python3-pip python3-dev \
    && rm -rf /var/lib/apt/lists/*

# 4. Python-пакеты, нужные системе сборки ArduPilot (empy 4.x несовместим, поэтому 3.3.4)
RUN pip3 install --no-cache-dir empy==3.3.4 future pexpect pymavlink dronecan

# 5. Исходники нужной версии
RUN git clone --depth 1 --branch ${ARDUPILOT_TAG} \
        --recurse-submodules --shallow-submodules \
        https://github.com/ArduPilot/ardupilot.git /ardupilot
WORKDIR /ardupilot

# 6. Сборка прошивки коптера под SITL
RUN ./waf configure --board sitl && ./waf copter

# 7. Порты: 5760 = SERIAL0 (наземная станция), 5762 = SERIAL1 (бортовой компьютер), 5763 = SERIAL2
EXPOSE 5760 5762 5763

# 8. Что запускать при старте контейнера
CMD ["./build/sitl/bin/arducopter", \
     "-S", "-w", \
     "--model", "+", \
     "--speedup", "1", \
     "--home", "42.3898,-71.1476,14,270", \
     "--defaults", "Tools/autotest/default_params/copter.parm"]
