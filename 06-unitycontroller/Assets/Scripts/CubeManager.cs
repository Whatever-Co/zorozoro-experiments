using System;
using System.Collections.Generic;
using UnityEngine;
using MQTTnet;
using ZLogger;
using Microsoft.Extensions.Logging;


public class CubeManager : IDisposable
{

    private static readonly ILogger<CubeManager> logger = LogManager.GetLogger<CubeManager>();

    private IApplicationMessagePublisher publisher;
    private Dictionary<string, Cube> cubes = new Dictionary<string, Cube>();


    public CubeManager(IApplicationMessagePublisher publisher)
    {
        this.publisher = publisher;
    }


    public Cube CreateCube(string address)
    {
        if (cubes.TryGetValue(address, out var cube))
        {
            return cube;
        }
        cube = new Cube(address, publisher);
        cubes.Add(address, cube);
        return cube;
    }


    public void SetLamp(Color32 color)
    {
        foreach (var (address, cube) in cubes)
        {
            cube.SetLamp(color);
        }
    }


    public void SetBattery(string address, int value)
    {
        logger.ZLogTrace("Setting battery of {0} to {1}", address, value);
        if (!cubes.ContainsKey(address))
        {
            logger.ZLogWarning("Address {0} not found.. adding...", address);
            CreateCube(address);
        }
        if (cubes.TryGetValue(address, out var cube))
        {
            cube.SetBattery(value);
        }
    }


    public void Dispose()
    {
    }

}
