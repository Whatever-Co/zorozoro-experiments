using Microsoft.Extensions.Logging;
using UnityEngine;
using UnityEngine.UI;
using ZLogger;


public class Main : MonoBehaviour
{

    private static readonly ILogger<Main> logger = LogManager.GetLogger<Main>();

    public Text statusText;

    private CubeManager cubeManager;
    private BridgeManager bridgeManager;


    void Start()
    {
        Application.targetFrameRate = 60;

        statusText.text = "Starting...";

        bridgeManager = gameObject.GetComponent<BridgeManager>();
        bridgeManager.OnMessage += HandleMessage;

        var scanner = GetComponent<ScannerReceiver>();
        scanner.OnNewCube += bridgeManager.ConnectToCube;

        cubeManager = CubeManager.Instance;
        cubeManager.BridgeManager = bridgeManager;
    }


    private void HandleMessage(string bridgeAddress, string cubeAddress, string command, byte[] payload)
    {
        Dispatcher.runOnUiThread(() =>
        {
            switch (command)
            {
                case "connected":
                    var cube = cubeManager.AddCube(cubeAddress, bridgeAddress);
                    logger.ZLogTrace(cube.ToString());
                    break;

                case "disconnected":
                    cubeManager.RemoveCube(cubeAddress);
                    break;

                case "position":
                    cubeManager.NotifyPosition(cubeAddress, payload);
                    break;

                case "battery":
                    cubeManager.NotifyBattery(cubeAddress, payload[0]);
                    break;
            }
        });
    }


    void Update()
    {
        if (Time.frameCount % 120 == 0)
        {
            statusText.text = $"{bridgeManager.BridgeCount} bridges, {cubeManager.CubeCount} cubes connected.";
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
            cubeManager.MoveForwardAll();
        }
        if (Input.GetKeyDown(KeyCode.DownArrow))
        {
            cubeManager.MoveBackwardAll();
        }
        if (Input.GetKeyDown(KeyCode.RightArrow))
        {
            cubeManager.RotateRightAll();
        }
        if (Input.GetKeyDown(KeyCode.LeftArrow))
        {
            cubeManager.RotateLeftAll();
        }
        if (Input.GetKeyUp(KeyCode.UpArrow) || Input.GetKeyUp(KeyCode.DownArrow) || Input.GetKeyUp(KeyCode.RightArrow) || Input.GetKeyUp(KeyCode.LeftArrow))
        {
            cubeManager.StopAll();
        }
    }


    public void ShowBatteryStatus()
    {
        cubeManager.ShowBatteryStatusAll();
    }


    static Color[] Colors = {
        new Color(1, 1, 0, 1),
        new Color(1, 0, 1, 1),
        new Color(0, 1, 1, 1)
    };
    static int count = 0;

    public void RandomColor()
    {
        cubeManager.SetLampAll(Colors[count++ % Colors.Length]);
    }


    int angle = 0;

    public void RandomRotate()
    {
        cubeManager.SetDirectionAll(angle);
        angle = (angle + 135) % 360;
    }


    public void LookCenter()
    {
        cubeManager.LookCenterAll();
    }


    public void GoAround()
    {
        cubeManager.GoAroundAll();
    }

}
