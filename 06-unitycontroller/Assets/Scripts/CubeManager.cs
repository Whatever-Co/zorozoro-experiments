using Microsoft.Extensions.Logging;
using MQTTnet;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using UnityEngine;
using ZLogger;


public class CubeManager : MonoBehaviour
{

    private static readonly ILogger<CubeManager> logger = LogManager.GetLogger<CubeManager>();


    public IApplicationMessagePublisher Publisher { get; set; }
    public Transform World;

    private Dictionary<string, Cube> cubes = new Dictionary<string, Cube>();


    public Cube AddOrGetCube(string address)
    {
        if (cubes.TryGetValue(address, out var cube))
        {
            return cube;
        }

        var prefab = Resources.Load<GameObject>("Prefabs/Cube");
        cube = UnityEngine.Object.Instantiate((prefab).GetComponent<Cube>());
        cube.name = address;
        cube.Init(address, this);
        cube.transform.SetParent(World, false);
        cubes.Add(address, cube);
        return cube;
    }


    public void SendMotor(string address, byte[] payload)
    {
        try
        {
            var message = new MqttApplicationMessageBuilder()
                .WithTopic(address + "/motor")
                .WithPayload(payload)
                .WithAtLeastOnceQoS()
                .Build();
            Publisher.PublishAsync(message, CancellationToken.None);
        }
        catch (System.Exception e)
        {
            Debug.LogException(e);
        }
    }


    public void SendLamp(string address, byte[] payload)
    {
        try
        {
            var message = new MqttApplicationMessageBuilder()
                .WithTopic(address + "/lamp")
                .WithPayload(payload)
                .WithAtLeastOnceQoS()
                .Build();
            Publisher.PublishAsync(message, CancellationToken.None);
        }
        catch (System.Exception e)
        {
            Debug.LogException(e);
        }
    }


    public void MoveForward()
    {
        foreach (var (address, cube) in cubes)
        {
            cube.MoveForward();
        }
    }


    public void MoveBackward()
    {
        foreach (var (address, cube) in cubes)
        {
            cube.MoveBackward();
        }
    }


    public void RotateRight()
    {
        foreach (var (address, cube) in cubes)
        {
            cube.RotateRight();
        }
    }


    public void RotateLeft()
    {
        foreach (var (address, cube) in cubes)
        {
            cube.RotateLeft();
        }
    }


    public void Stop()
    {
        foreach (var (address, cube) in cubes)
        {
            cube.Stop();
        }
    }


    public void NotifyPosition(string address, byte[] data)
    {
        var cube = AddOrGetCube(address);
        cube.NotifyPosition(data);
    }


    public void SetDirection(int angle)
    {
        foreach (var (address, cube) in cubes)
        {
            cube.SetDirection(angle);
        }
    }


    public void SetLamp(Color32 color)
    {
        foreach (var (address, cube) in cubes)
        {
            cube.SetLamp(color);
        }
    }


    public void ShowBatteryStatus()
    {
        foreach (var (address, cube) in cubes)
        {
            cube.ShowBatteryStatus();
        }
    }


    public void NotifyBattery(string address, int value)
    {
        // logger.ZLogTrace("Setting battery of {0} to {1}", address, value);
        var cube = AddOrGetCube(address);
        cube.NotifyBattery(value);
    }


    public void LookCenter()
    {
        foreach (var (address, cube) in cubes)
        {
            cube.LookCenter();
        }
    }


    public void GoAround()
    {
        foreach (var (address, cube) in cubes)
        {
            cube.EnableGoAround();
        }
    }


    public int ConnectedCubeCount
    {
        get
        {
            var t = Time.realtimeSinceStartup;
            return cubes.Where(kv => t - kv.Value.LastBatteryTime < 8f).Count();
        }

    }

}
