using Microsoft.Extensions.Logging;
using MQTTnet;
using System.Collections.Generic;
using System.Linq;
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
        cube.Init(address, Publisher);
        cube.transform.SetParent(World, false);
        cubes.Add(address, cube);
        return cube;
    }


    public void SetPosition(string address, byte[] data)
    {
        var cube = AddOrGetCube(address);
        cube.SetPosition(data);
    }


    public void SetMotorAll(int angle)
    {
        foreach (var (address, cube) in cubes)
        {
            cube.SetMotor(angle);
        }
    }


    public void SetLampAll(Color32 color)
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


    public void SetBattery(string address, int value)
    {
        logger.ZLogTrace("Setting battery of {0} to {1}", address, value);
        var cube = AddOrGetCube(address);
        cube.SetBattery(value);
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
            return cubes.Where(kv => t - kv.Value.LastPositionTime < 1f).Count();
        }

    }

}
