"""Проверка двусторонней связи с ArduPilot SITL через pymavlink.

1. Подключается к SITL по TCP и ждёт HEARTBEAT (дрон -> мы).
2. Отправляет команду MAV_CMD_SET_MESSAGE_INTERVAL (мы -> дрон).
3. Ждёт COMMAND_ACK: дрон подтвердил команду.
4. Печатает несколько сообщений ATTITUDE: дрон присылает запрошенную телеметрию.

Запуск:  .venv/bin/python src/test_mavlink.py
"""

import math
import sys

from pymavlink import mavutil

CONNECTION = "tcp:127.0.0.1:5762"
RATE_HZ = 10           # сколько раз в секунду присылать ATTITUDE
SAMPLES = 5            # сколько сообщƒений ATTITUDE напечатать
TIMEOUT_S = 5          # сколько секунд ждать каждого ответа


def fail(text: str) -> None:
    print(f"ОШИБКА: {text}")
    sys.exit(1)


def main() -> None:
    print(f"Подключаюсь к {CONNECTION} ...")
    master = mavutil.mavlink_connection(CONNECTION)

    # 1. HEARTBEAT
    if master.wait_heartbeat(timeout=TIMEOUT_S) is None:
        fail(f"нет HEARTBEAT за {TIMEOUT_S} с — SITL запущен? к порту 5760 кто-то подключён?")
    print(f"HEARTBEAT: sysid={master.target_system} compid={master.target_component}")

    # 2. Запрос телеметрии: param1 = ID сообщения, param2 = интервал в микросекундах
    interval_us = 1_000_000 // RATE_HZ
    master.mav.command_long_send(
        master.target_system,
        master.target_component,
        mavutil.mavlink.MAV_CMD_SET_MESSAGE_INTERVAL,
        0,                                           # confirmation: первая попытка
        mavutil.mavlink.MAVLINK_MSG_ID_ATTITUDE,     # param1
        interval_us,                                 # param2
        0, 0, 0, 0, 0,                               # param3..param7 не используются
    )
    print(f"Отправил SET_MESSAGE_INTERVAL: ATTITUDE каждые {interval_us} мкс ({RATE_HZ} Гц)")

    # 3. Подтверждение
    ack = master.recv_match(type="COMMAND_ACK", blocking=True, timeout=TIMEOUT_S)
    if ack is None:
        fail(f"нет COMMAND_ACK за {TIMEOUT_S} с")
    if ack.result != mavutil.mavlink.MAV_RESULT_ACCEPTED:
        fail(f"команду отклонили, result={ack.result}")
    print("COMMAND_ACK: принято")

    # 4. Телеметрия
    for i in range(1, SAMPLES + 1):
        msg = master.recv_match(type="ATTITUDE", blocking=True, timeout=TIMEOUT_S)
        if msg is None:
            fail(f"нет ATTITUDE за {TIMEOUT_S} с")
        print(
            f"ATTITUDE #{i}: "
            f"roll={math.degrees(msg.roll):6.2f}°  "
            f"pitch={math.degrees(msg.pitch):6.2f}°  "
            f"yaw={math.degrees(msg.yaw):7.2f}°"
        )

    print("Связь в обе стороны работает ✅")


if __name__ == "__main__":
    main()
