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

    public static readonly int MAX_CUBES_PER_BRIDGE = 8;


    public string Id { get; }
    public int NumConnectedCubes { get => cubes?.Count ?? 0; }
    public bool IsBusy { get; private set; } = false;
    public BridgeInfo InfoPanel
    {
        get => info; set
        {
            info = value;
            UpdateInfo();
        }
    }

    private Cube connectingCube;
    private Dictionary<string, Cube> cubes = new Dictionary<string, Cube>();
    private IApplicationMessagePublisher publisher;
    private BridgeInfo info;


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
            cubes.Add(address, connectingCube);
            Debug.Log($"new cube {address} added");
            connectingCube.SetLamp(Color.white);
        }
        else
        {
            Debug.LogWarning("???");
        }
        connectingCube = null;
        UpdateInfo();
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
            if (cubes.ContainsKey(address))
            {
                cubes.Remove(address);
                Debug.Log("cube removed " + address);
            }
        }
        UpdateInfo();
    }


    private void HandleBattery(string payload)
    {
        var t = payload.Split(',');
        if (int.TryParse(t[1], out var value))
        {
            var address = t[0];
            if (cubes.TryGetValue(address, out var cube))
            {
                cube.SetBattery(value);
            }
            else
            {
                Debug.LogWarning($"unknown cube {address}");
            }
        }
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

                publisher.PublishAsync(new MqttApplicationMessage
                {
                    Topic = $"{Id}/lamp",
                    Payload = stream.ToArray(),
                }, CancellationToken.None);
            }
        }
        catch (System.Exception e)
        {
            Debug.LogException(e);
        }
    }


    void UpdateInfo()
    {
        Dispatcher.runOnUiThread(() =>
        {
            info?.SetAddress(Id);
            info?.SetCubes(cubes.Select(x => x.Value).ToArray());
        });
    }


    public void Dispose()
    {
    }

}
