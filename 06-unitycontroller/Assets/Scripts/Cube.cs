using Microsoft.Extensions.Logging;
using MQTTnet;
using System.IO;
using System.Threading;
using UnityEngine;
using ZLogger;


public class Cube : MonoBehaviour
{

    public string Address { get; private set; }
    public bool IsConnected { get; private set; }

    public int Battery { get; private set; } = -1;

    private IApplicationMessagePublisher publisher;

    // private Vector3 currentPosition;
    // private Quaternion currentRotation;



    void Update()
    {
        // transform.localPosition = Vector3.Lerp(transform.localPosition, currentPosition, 0.5f);
        // transform.localRotation = Quaternion.Slerp(transform.localRotation, currentRotation, 0.5f);
    }


    public void Init(string address, IApplicationMessagePublisher publisher)
    {
        Address = address;
        IsConnected = false;
        this.publisher = publisher;
    }


    public void SetMotor()
    {
        byte[] data = { 0x02,
                        0x01, 0x01, 50,
                        0x02, 0x02, 50,
                        100 };
        var message = new MqttApplicationMessageBuilder()
            .WithTopic(Address + "/motor")
            .WithPayload(data)
            .Build();
        publisher.PublishAsync(message, CancellationToken.None);
    }


    public void SetPosition(byte[] data)
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
                    var centerRotation = reader.ReadUInt16();
                    // var sensorX = reader.ReadUInt16();
                    // var sensorY = reader.ReadUInt16();
                    // var sensorRotation = reader.ReadUInt16();
                    transform.localPosition = new Vector3(centerX, 0, -centerY);
                    transform.localRotation = Quaternion.Euler(0, centerRotation, 0);
                    break;
                case 0x02: // Standard ID
                    break;
                case 0x03: // Position ID missed
                    break;
                case 0x04: // Standard ID missed
                    break;
            }
        }
    }


    public void SetLamp(byte r, byte g, byte b)
    {
        byte[] data = { 0x03, 0x00, 0x01, 0x01, r, g, b };
        WriteLampCommand(data);
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
        WriteLampCommand(data);
    }


    public void SetLampBlink(Color32 color, int interval)
    {
        SetLampBlink(color.r, color.g, color.b, interval);
    }


    public void WriteLampCommand(byte[] data)
    {
        var message = new MqttApplicationMessageBuilder()
            .WithTopic(Address + "/lamp")
            .WithPayload(data)
            .Build();
        publisher.PublishAsync(message, CancellationToken.None);
    }


    public void SetBattery(int battery)
    {
        if (Battery == battery)
        {
            return;
        }

        Battery = battery;
        if (battery <= 10)
        {
            SetLampBlink(Color.red, 300);
        }
        else if (battery <= 20)
        {
            SetLamp(Color.red);
        }
        else if (battery <= 50)
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
