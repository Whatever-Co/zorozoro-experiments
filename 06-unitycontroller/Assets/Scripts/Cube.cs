using Microsoft.Extensions.Logging;
using MQTTnet;
using MQTTnet.Protocol;
using System.Collections.Generic;
using System.Linq;
using System.IO;
using System.Text;
using System.Threading;
using UnityEngine;
using ZLogger;


public class Cube
{

    public string Address { get; }
    public bool IsConnected { get; set; }
    public Bridge Bridge { get; set; }

    public int Battery { get; private set; } = -1;

    private IApplicationMessagePublisher publisher;


    public Cube(string address, IApplicationMessagePublisher publisher)
    {
        Address = address;
        IsConnected = false;
        this.publisher = publisher;
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



    public override string ToString() => $"[Cube Address={Address} IsConnected={IsConnected}]";

}
