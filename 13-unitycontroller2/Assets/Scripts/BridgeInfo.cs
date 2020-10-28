using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.UI;


public class BridgeInfo : MonoBehaviour
{

    Text address;
    Text cubes;


    void Awake()
    {
        address = transform.Find("Address").GetComponent<Text>();
        address.text = "";
        cubes = transform.Find("Cubes").GetComponent<Text>();
        cubes.text = "";
    }


    public void SetAddress(string address)
    {
        this.address.text = address;
        gameObject.name = address;
    }


    public void SetCubes(Cube[] cubes)
    {
        this.cubes.text = $"{cubes.Length} cubes\n";
        this.cubes.text += string.Join("\n", cubes.Select(cube => cube.Address));
    }

}
