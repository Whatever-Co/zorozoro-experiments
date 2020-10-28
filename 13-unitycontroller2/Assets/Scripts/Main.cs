using Microsoft.Extensions.Logging;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Connecting;
using MQTTnet.Client.Disconnecting;
using MQTTnet.Client.Options;
using MQTTnet.Client.Receiving;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using UnityEngine;
using UnityEngine.UI;
using ZLogger;


public class Main : MonoBehaviour, IMqttClientConnectedHandler, IMqttClientDisconnectedHandler, IMqttApplicationMessageReceivedHandler
{

    private static readonly ILogger<Main> logger = LogManager.GetLogger<Main>();

    public Text statusText;

    private CubeManager cubeManager;
    private List<Bridge> bridges;

    // private Server server;
    private IMqttClientOptions options;
    private IMqttClient client;

    private TcpServer tcp;


    async void Start()
    {
        Application.targetFrameRate = 60;

        statusText.text = "Starting...";

        // server = new Server();
        // await server.Start();

        client = new MqttFactory().CreateMqttClient();
        client.UseConnectedHandler(this);
        client.UseDisconnectedHandler(this);
        client.UseApplicationMessageReceivedHandler(this);

        options = new MqttClientOptionsBuilder()
            .WithClientId("controller")
            .WithTcpServer("localhost")
            .WithCleanSession()
            .Build();
        await client.ConnectAsync(options);

        tcp = gameObject.GetComponent<TcpServer>();
        tcp.Connected += OnConnected;

        bridges = new List<Bridge>();

        cubeManager = GetComponent<CubeManager>();
        cubeManager.Publisher = client;
        cubeManager.TcpServer = tcp;
    }


    public Task HandleConnectedAsync(MqttClientConnectedEventArgs eventArgs)
    {
        logger.ZLogDebug("Connected");
        return Task.Run(async () =>
        {
            var topics = new[] { "newcube", "+/available", "+/connected", "+/disconnected", "+/position", "+/button", "+/battery" };
            foreach (var t in topics)
            {
                await client.SubscribeAsync(new MqttTopicFilterBuilder().WithTopic(t).Build());
                logger.ZLogDebug("Subscribed to topic: " + t);
            }
        });
    }


    public Task HandleDisconnectedAsync(MqttClientDisconnectedEventArgs eventArgs)
    {
        return Task.Run(async () =>
        {
            logger.ZLogDebug("### DISCONNECTED FROM SERVER ###");
            await Task.Delay(System.TimeSpan.FromSeconds(5));
            try
            {
                await client.ConnectAsync(options, CancellationToken.None);
            }
            catch
            {
                logger.ZLogDebug("### RECONNECTING FAILED ###");
            }
        });
    }


    public Task HandleApplicationMessageReceivedAsync(MqttApplicationMessageReceivedEventArgs e)
    {
        Dispatcher.runOnUiThread(() =>
        {
            var m = e.ApplicationMessage;
            var payload = m.Payload != null ? Encoding.UTF8.GetString(m.Payload) : "";
            logger.ZLogDebug($"Message received: Client = {e.ClientId}, Topic = {m.Topic}, Payload = {payload}");
            switch (m.Topic)
            {
                case "newcube":
                    var bridge = bridges.Where(b => !b.ConnectingCube).OrderByDescending(b => b.AvailableSlot).FirstOrDefault();
                    Debug.LogWarning(bridge);
                    bridge?.ConnectToCube(m.Payload);
                    return;
            }
        });
        return null;
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


    async void OnApplicationQuit()
    {
        foreach (var bridge in bridges)
        {
            bridge.Stop();
        }
        await client.DisconnectAsync();
        client.Dispose();
        client = null;
        // await server.Stop();
        Debug.LogWarning("Application Quit");
    }

}
