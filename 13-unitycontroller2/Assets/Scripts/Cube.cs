using Microsoft.Extensions.Logging;
using System.Collections;
using System.IO;
using UnityEngine;
using ZLogger;


public class Cube : MonoBehaviour
{

    public static readonly Vector2 MAT_MIN = new Vector2(98, 142);
    public static readonly Vector2 MAT_MAX = new Vector2(402, 358);
    public static readonly Vector2 MAT_CENTER = (MAT_MIN + MAT_MAX) / 2;

    public static readonly float DOTS_PER_METER = 411f / 0.560f; // 411/0.560 dot/m


    public string Address { get; private set; }
    public bool IsConnected { get; private set; }

    public int Battery { get; private set; } = -1;
    public float LastBatteryTime { get; private set; } = 0;

    public Bridge Bridge { get; private set; }

    private Vector2 currentMatPosition;
    public float LastPositionTime { get; private set; } = 0;
    private bool goingAroundNow = false;
    public bool IsOnSheet
    {
        get => Vector2.Distance(currentMatPosition, Vector2.zero) > Vector2.kEpsilon;
    }


    public void Init(string address, Bridge bridge)
    {
        Address = address;
        Bridge = bridge;
        IsConnected = false;
    }


    void OnDestroy()
    {
        Bridge = null;
    }


    public void MoveForward(byte speed = 10, byte timeout = 255)
    {
        DisableGoAround();

        byte[] data = { 0x02, 0x01, 0x01, speed, 0x02, 0x01, speed, timeout };
        Bridge.SendMotor(Address, data);
    }


    public void MoveBackward(byte speed = 10, byte timeout = 255)
    {
        DisableGoAround();

        byte[] data = { 0x02, 0x01, 0x02, speed, 0x02, 0x02, speed, timeout };
        Bridge.SendMotor(Address, data);
    }


    public void RotateRight(byte speed = 10, byte timeout = 255)
    {
        DisableGoAround();

        byte[] data = { 0x02, 0x01, 0x01, speed, 0x02, 0x02, speed, timeout };
        Bridge.SendMotor(Address, data);
    }


    public void RotateLeft(byte speed = 10, byte timeout = 255)
    {
        DisableGoAround();

        byte[] data = { 0x02, 0x01, 0x02, speed, 0x02, 0x01, speed, timeout };
        Bridge.SendMotor(Address, data);
    }


    public void Stop()
    {
        DisableGoAround();

        byte[] data = { 0x01, 0x01, 0x01, 0, 0x02, 0x01, 0 };
        Bridge.SendMotor(Address, data);
    }


    public void SetDirection(int angle)
    {
        DisableGoAround();

        using (var stream = new MemoryStream())
        using (var writer = new BinaryWriter(stream))
        {
            byte[] data = { 0x03, 0x00, 5, 0, 100, 0, 0x00 };
            writer.Write(data);
            writer.Write((ushort)0xffff);
            writer.Write((ushort)0xffff);
            writer.Write((ushort)((0x00 << 13) | angle));
            Bridge.SendMotor(Address, stream.ToArray());
        }
    }


    public void LookCenter()
    {
        if (!IsOnSheet)
        {
            return;
        }
        var p = currentMatPosition - MAT_CENTER;
        var a = Mathf.Atan2(p.y, p.x) * Mathf.Rad2Deg;
        SetDirection(Mathf.RoundToInt(a + 360 + 90));
    }


    public void EnableGoAround()
    {
        if (goingAroundNow)
        {
            return;
        }
        goingAroundNow = true;

        Debug.LogWarning($"currentMatPosition={currentMatPosition} dist={Vector2.Distance(currentMatPosition, Vector2.zero)}, LastPositionTime={LastPositionTime}, dt={Time.realtimeSinceStartup - LastPositionTime}");

        if (IsOnSheet)
        {
            StartGoAround();
        }
    }


    public void DisableGoAround()
    {
        StopGoAround();
        goingAroundNow = false;
    }


    float radius;
    byte speed;
    Coroutine coroutine = null;

    private void StartGoAround()
    {
        if (coroutine != null)
        {
            return;
        }

        radius = Mathf.Clamp(Vector2.Distance(currentMatPosition, MAT_CENTER), 30, 90);
        // radius = 90;
        speed = (byte)Mathf.FloorToInt(radius / 5f);
        radius = speed * 5;
        Debug.LogWarning($"radius={radius}, speed={speed}");
        coroutine = StartCoroutine(_GoAround());
    }


    private void StopGoAround()
    {
        if (coroutine != null)
        {
            StopCoroutine(coroutine);
            coroutine = null;
        }
    }

    IEnumerator _GoAround()
    {
        while (true)
        {
            // const float DIST = 100f;
            const float A = 60f;
            // L = 2 PI R
            // var dist = 2 * Mathf.PI * radius * A / 360f;
            // float A = DIST / (2 * Mathf.PI * radius) * 360;
            // Debug.LogWarning($"radius={radius}, DIST={DIST}, A={A}");

            using (var stream = new MemoryStream())
            using (var writer = new BinaryWriter(stream))
            {
                byte[] data = { 0x04, 0x66, 100, 1, speed, 0, 0x00, 0 };
                writer.Write(data);

                var p = currentMatPosition - MAT_CENTER;
                var startAngle = Mathf.Atan2(p.y, p.x);
                Debug.LogWarning($"t={Time.realtimeSinceStartup}, a={startAngle * Mathf.Rad2Deg}");
                // var radius = Vector2.Distance(currentMatPosition, MAT_CENTER);
                for (int i = 1; i <= 2; i++)
                {
                    var a = startAngle + i / 2f * A * Mathf.Deg2Rad;
                    ushort x = (ushort)(Mathf.Cos(a) * radius + MAT_CENTER.x);
                    ushort y = (ushort)(Mathf.Sin(a) * radius + MAT_CENTER.y);
                    Debug.LogWarning($"a={a * Mathf.Rad2Deg}, x={x}, y={y}");
                    writer.Write(x);
                    writer.Write(y);
                    writer.Write((ushort)(0x05 << 13));
                }
                Bridge.SendMotor(Address, stream.ToArray());
            }

            yield return new WaitForSeconds(2f);
        }
    }


    public void NotifyPosition(byte[] data)
    {
        using (var stream = new MemoryStream(data))
        using (var reader = new BinaryReader(stream))
        {
            var type = reader.ReadByte();
            switch (type)
            {
                case 0x01: // Position ID
                    var centerX = reader.ReadUInt16();
                    var centerY = reader.ReadUInt16();
                    currentMatPosition.x = centerX;
                    currentMatPosition.y = centerY;
                    var centerRotation = reader.ReadUInt16();
                    // var sensorX = reader.ReadUInt16();
                    // var sensorY = reader.ReadUInt16();
                    // var sensorRotation = reader.ReadUInt16();
                    transform.localPosition = new Vector3(centerX, 0, -centerY);
                    transform.localRotation = Quaternion.Euler(0, centerRotation, 0);

                    GetComponentInChildren<MeshRenderer>().enabled = true;

                    LastPositionTime = Time.realtimeSinceStartup;
                    if (goingAroundNow)
                    {
                        StartGoAround();
                    }
                    break;

                case 0x02: // Standard ID
                    break;

                case 0x03: // Position ID missed
                    currentMatPosition = Vector2.zero;
                    GetComponentInChildren<MeshRenderer>().enabled = false;
                    LastPositionTime = Time.realtimeSinceStartup;
                    StopGoAround();
                    break;

                case 0x04: // Standard ID missed
                    break;
            }
        }
    }


    public void SetLamp(byte r, byte g, byte b)
    {
        byte[] data = { 0x03, 0x00, 0x01, 0x01, r, g, b };
        Bridge.SendLamp(Address, data);
    }


    public void SetLamp(Color32 color)
    {
        SetLamp(color.r, color.g, color.b);
    }


    public void SetLampBlink(byte r, byte g, byte b, int interval)
    {
        byte duration = (byte)Mathf.Clamp(interval / 10, 1, 255);
        byte[] data = { 0x04, 0x00, 0x02,
                        duration, 0x01, 0x01, r, g, b,
                        duration, 0x01, 0x01, 0, 0, 0 };
        Bridge.SendLamp(Address, data);
    }


    public void SetLampBlink(Color32 color, int interval)
    {
        SetLampBlink(color.r, color.g, color.b, interval);
    }


    public void NotifyBattery(int battery)
    {
        LastBatteryTime = Time.realtimeSinceStartup;
        if (Battery == battery)
        {
            return;
        }
        Battery = battery;
        ShowBatteryStatus();
    }


    public void ShowBatteryStatus()
    {
        if (Battery <= 10)
        {
            SetLampBlink(Color.red, 300);
        }
        else if (Battery <= 20)
        {
            SetLamp(Color.red);
        }
        else if (Battery <= 50)
        {
            SetLamp(254, 176, 25);
        }
        else
        {
            SetLamp(Color.green);
        }
    }


    public override string ToString() => $"[Cube Address={Address} IsConnected={IsConnected}]";

}
