using Microsoft.Extensions.Logging;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.UI;
using ZLogger;


public class Main : MonoBehaviour
{

    private static readonly ILogger<Main> logger = LogManager.GetLogger<Main>();

    public Text statusText;

    private CubeManager cubeManager;
    private List<Bridge> bridges;

    private TcpServer tcp;


    void Start()
    {
        Application.targetFrameRate = 60;

        statusText.text = "Starting...";

        tcp = gameObject.GetComponent<TcpServer>();
        tcp.Connected += OnConnected;

        var scanner = GetComponent<ScannerReceiver>();
        scanner.OnNewCube += OnNewCube;

        bridges = new List<Bridge>();

        cubeManager = GetComponent<CubeManager>();
        cubeManager.TcpServer = tcp;
    }


    private void OnNewCube(string address)
    {
        var bridge = bridges.Where(b => !b.ConnectingCube).OrderByDescending(b => b.AvailableSlot).FirstOrDefault();
        // Debug.LogWarning(bridge);
        bridge?.ConnectToCube(address);
    }


    private void OnConnected(Bridge bridge)
    {
        Debug.Log("OnConnected");
        bridge.OnMessage += OnMessage;
        bridge.OnDisconnected += OnDisconnected;
        bridge.Start();
        bridges.Add(bridge);
    }


    private void OnDisconnected(Bridge bridge)
    {
        Debug.Log("OnDisconnected");
        bridge.OnDisconnected -= OnDisconnected;
        bridges.Remove(bridge);
    }


    private void OnMessage(Bridge bridge, string address, string command, byte[] payload)
    {
        Dispatcher.runOnUiThread(() =>
        {
            switch (command)
            {
                case "connected":
                    var cube = cubeManager.AddCube(address, bridge);
                    Debug.Log(cube);
                    break;

                case "disconnected":
                    cubeManager.RemoveCube(address);
                    break;

                case "position":
                    cubeManager.NotifyPosition(address, payload);
                    break;

                case "battery":
                    cubeManager.NotifyBattery(address, payload[0]);
                    break;
            }
        });
    }


    void Update()
    {
        if (Time.frameCount % 120 == 0)
        {
            statusText.text = $"{cubeManager.ConnectedCubeCount} cubes connected.";
        }

        if (Input.GetKeyDown(KeyCode.Alpha1))
        {
            RandomRotate();
        }
        if (Input.GetKeyDown(KeyCode.Alpha2))
        {
            RandomColor();
        }
        if (Input.GetKeyDown(KeyCode.Alpha3))
        {
            LookCenter();
        }
        if (Input.GetKeyDown(KeyCode.Alpha4))
        {
            GoAround();
        }

        if (Input.GetKeyDown(KeyCode.UpArrow))
        {
            cubeManager.MoveForward();
        }
        if (Input.GetKeyDown(KeyCode.DownArrow))
        {
            cubeManager.MoveBackward();
        }
        if (Input.GetKeyDown(KeyCode.RightArrow))
        {
            cubeManager.RotateRight();
        }
        if (Input.GetKeyDown(KeyCode.LeftArrow))
        {
            cubeManager.RotateLeft();
        }
        if (Input.GetKeyUp(KeyCode.UpArrow) || Input.GetKeyUp(KeyCode.DownArrow) || Input.GetKeyUp(KeyCode.RightArrow) || Input.GetKeyUp(KeyCode.LeftArrow))
        {
            cubeManager.Stop();
        }
    }


    public void ShowBatteryStatus()
    {
        cubeManager.ShowBatteryStatus();
    }


    static Color[] Colors = {
        new Color(1, 1, 0, 1),
        new Color(1, 0, 1, 1),
        new Color(0, 1, 1, 1)
    };
    static int count = 0;

    public void RandomColor()
    {
        cubeManager.SetLamp(Colors[count++ % Colors.Length]);
    }


    int angle = 0;

    public void RandomRotate()
    {
        cubeManager.SetDirection(angle);
        angle = (angle + 135) % 360;
    }


    public void LookCenter()
    {
        cubeManager.LookCenter();
    }


    public void GoAround()
    {
        cubeManager.GoAround();
    }


    void OnApplicationQuit()
    {
        foreach (var bridge in bridges)
        {
            bridge.Stop();
        }
        Debug.LogWarning("Application Quit");
    }

}
