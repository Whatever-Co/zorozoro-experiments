using Microsoft.Extensions.Logging;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using ZLogger;


public class CubeManager : MonoBehaviour
{

    private static readonly ILogger<CubeManager> logger = LogManager.GetLogger<CubeManager>();

    public static CubeManager Instance { get; private set; }


    public BridgeManager BridgeManager { get; set; }
    public Transform World;

    private Dictionary<string, Cube> cubes = new Dictionary<string, Cube>();

    public int CubeCount { get => cubes.Count(); }
    public bool GoAroundMode { get; private set; } = false;


    void Awake()
    {
        Instance = this;
    }


    public Cube AddCube(string cubeAddress, string bridgeAddress)
    {
        if (cubes.TryGetValue(cubeAddress, out var cube))
        {
            cube.BridgeAddress = bridgeAddress;
            cube.Battery = -1;
            return cube;
        }

        var prefab = Resources.Load<GameObject>("Prefabs/Cube");
        cube = UnityEngine.Object.Instantiate((prefab).GetComponent<Cube>());
        cube.Init(cubeAddress, bridgeAddress, BridgeManager);
        cube.transform.SetParent(World, false);
        cubes.Add(cubeAddress, cube);
        return cube;
    }


    public Cube GetCube(string address)
    {
        if (cubes.TryGetValue(address, out var cube))
        {
            return cube;
        }
        return null;
    }


    public void RemoveCube(string address)
    {
        var cube = GetCube(address);
        if (cube != null)
        {
            cubes.Remove(address);
            Destroy(cube.gameObject);
        }
    }


    public void NotifyPosition(string address, byte[] data)
    {
        var cube = GetCube(address);
        cube?.NotifyPosition(data);
    }


    public void NotifyBattery(string address, int value)
    {
        var cube = GetCube(address);
        cube?.NotifyBattery(value);
    }


    public void MoveForwardAll()
    {
        foreach (var (address, cube) in cubes)
        {
            cube.MoveForward();
        }
        GoAroundMode = false;
    }


    public void MoveBackwardAll()
    {
        foreach (var (address, cube) in cubes)
        {
            cube.MoveBackward();
        }
        GoAroundMode = false;
    }


    public void RotateRightAll()
    {
        foreach (var (address, cube) in cubes)
        {
            cube.RotateRight();
        }
        GoAroundMode = false;
    }


    public void RotateLeftAll()
    {
        foreach (var (address, cube) in cubes)
        {
            cube.RotateLeft();
        }
        GoAroundMode = false;
    }


    public void StopAll()
    {
        foreach (var (address, cube) in cubes)
        {
            cube.Stop();
        }
        GoAroundMode = false;
    }


    public void SetDirectionAll(int angle)
    {
        foreach (var (address, cube) in cubes)
        {
            cube.SetDirection(angle);
        }
        GoAroundMode = false;
    }


    public void SetLampAll(Color32 color)
    {
        foreach (var (address, cube) in cubes)
        {
            cube.SetLamp(color);
        }
    }


    public void ShowBatteryStatusAll()
    {
        foreach (var (address, cube) in cubes)
        {
            cube.ShowBatteryStatus();
        }
    }


    public void LookCenterAll()
    {
        foreach (var (address, cube) in cubes)
        {
            cube.LookCenter();
        }
        GoAroundMode = false;
    }


    public void GoAroundAll()
    {
        foreach (var (address, cube) in cubes)
        {
            cube.EnableGoAround();
        }
        GoAroundMode = true;
    }

}
