using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;


public class CubeManager : IDisposable
{

    private List<Cube> cubes = new List<Cube>();


    public CubeManager()
    {
    }


    public Cube CreateCube(string address)
    {
        var cube = cubes.Where(c => c.Address == address).FirstOrDefault();
        if (cube == null)
        {
            cube = new Cube(address);
            cubes.Add(cube);
        }
        return cube;
    }


    public void SetLamp(Color32 color)
    {
        foreach (var cube in cubes)
        {
            cube.SetLamp(color);
        }
    }


    public void Dispose()
    {
    }

}
