using System.Collections.Generic;
using UnityEngine;
using MQTTnet;
using ZLogger;
using Microsoft.Extensions.Logging;


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


    int angle = 0;


    public void SetMotorAll()
    {
        var a = Random.Range(0, 8) * 45;
        foreach (var (address, cube) in cubes)
        {
            cube.SetMotor(a);
            angle += 90;
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

}
