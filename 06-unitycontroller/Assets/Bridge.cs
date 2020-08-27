using MQTTnet;
using MQTTnet.Protocol;
using System.Collections.Generic;
using System.Linq;
using System.IO;
using System.Text;
using System.Threading;
using UnityEngine;


public class Bridge : System.IDisposable
{

    public string Id { get; }
    public int NumConnectedCubes { get => cubes?.Count ?? 0; }
    public bool IsBusy { get; private set; } = false;

    private Cube connectingCube;
    private List<Cube> cubes = new List<Cube>();
    private IApplicationMessagePublisher publisher;


    public Bridge(string id, IApplicationMessagePublisher publisher)
    {
        this.Id = id;
        this.publisher = publisher;
    }


    public void Connect(Cube cube)
    {
        Debug.Log($"connecting {cube.Address} through {Id}");
        IsBusy = true;
        connectingCube = cube;

        publisher.PublishAsync(new MqttApplicationMessage
        {
            Topic = $"{Id}/connect",
            Payload = Encoding.ASCII.GetBytes(cube.Address),
            QualityOfServiceLevel = MqttQualityOfServiceLevel.AtMostOnce,
        }, CancellationToken.None);
    }


    public void ProcessPayload(string topic, string payload)
    {
        Debug.Log($"ProcessPayload: {Id}, {topic}, {payload}");
        switch (topic)
        {
            case "connected":
                HandleConnected(payload);
                break;
            case "disconnected":
                HandleDisconnected(payload);
                break;
            case "battery":
                HandleBattery(payload);
                break;
        }
    }


    private void HandleConnected(string address)
    {
        IsBusy = false;
        if (connectingCube.Address == address)
        {
            connectingCube.Bridge = this;
            cubes.Add(connectingCube);
            Debug.Log($"new cube {address} added");
            connectingCube.SetLamp();
        }
        else
        {
            Debug.LogWarning("???");
        }
        connectingCube = null;
    }


    private void HandleDisconnected(string address)
    {
        IsBusy = false;
        if (connectingCube != null)
        {
            Debug.Log($"connect failed {connectingCube.Address}");
            connectingCube = null;
        }
        else
        {
            var remove = cubes.Where(c => c.Address == address && c.Bridge == this).ToList();
            foreach (var c in remove)
            {
                cubes.Remove(c);
                Debug.Log("cube removed " + c.Address);
            }
        }
    }


    private void HandleBattery(string payload)
    {
        // if (int.TryParse(battery, out var value))
        // {
        //     Debug.Log($"");
        // }
    }


    public void WriteLampCommand(Cube cube, byte[] data)
    {
        try
        {
            using (var stream = new MemoryStream())
            using (var writer = new BinaryWriter(stream))
            {
                writer.Write(Encoding.ASCII.GetBytes(cube.Address));
                writer.Write((byte)data.Length);
                writer.Write(data);

                var buffer = stream.ToArray();
                Debug.LogWarning($"{cube.Address}, {cube.Address.Length}, {data.Length}, {buffer.Length}");
                publisher.PublishAsync(new MqttApplicationMessage
                {
                    Topic = $"{Id}/lamp",
                    Payload = buffer,
                }, CancellationToken.None);
            }
        }
        catch (System.Exception e)
        {
            Debug.LogException(e);
        }
    }


    public void Dispose()
    {
    }

}
