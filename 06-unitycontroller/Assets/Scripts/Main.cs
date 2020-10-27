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
    private Dictionary<string, Bridge> availableBridegs;

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

        cubeManager = GetComponent<CubeManager>();
        cubeManager.Publisher = client;

        availableBridegs = new Dictionary<string, Bridge>();

        options = new MqttClientOptionsBuilder()
            .WithClientId("controller")
            .WithTcpServer("localhost")
            .WithCleanSession()
            .Build();
        await client.ConnectAsync(options);

        tcp = gameObject.GetComponent<TcpServer>();
        tcp.UseApplicationMessageReceivedHandler(this);

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
                    var value = availableBridegs.OrderByDescending(x => x.Value.AvailableSlot).FirstOrDefault();
                    Debug.LogWarning(value);
                    if (!string.IsNullOrEmpty(value.Key))
                    {
                        availableBridegs.Remove(value.Key);
                        var message = new MqttApplicationMessageBuilder()
                            .WithTopic($"{value.Key}/newcube")
                            .WithPayload(m.Payload)
                            .WithAtLeastOnceQoS()
                            .Build();
                        client.PublishAsync(message, CancellationToken.None);
                        tcp.Publish(message);
                    }
                    return;
            }
            var t = m.Topic.Split('/');
            var address = t[0];
            switch (t[1])
            {
                case "available":
                    logger.ZLogTrace("available client: {0} / {1}", address, m.Payload[0]);
                    if (availableBridegs.ContainsKey(address))
                    {
                        availableBridegs[address].AvailableSlot = m.Payload[0];
                    }
                    else
                    {
                        var bridge = new Bridge(address) { AvailableSlot = m.Payload[0] };
                        availableBridegs.Add(address, bridge);
                    }
                    break;

                case "connected":
                    var cube = cubeManager.AddOrGetCube(address);
                    cube.SetLamp(Color.white);
                    break;

                case "disconnected":
                    break;

                case "position":
                    cubeManager.NotifyPosition(address, m.Payload);
                    break;

                case "button":
                    // byte state = m.Payload[0];
                    // if (state > 0)
                    // {
                    //     // var color = new Color(Random.value, Random.value, Random.value);
                    //     // logger.ZLogDebug(color.ToString());
                    //     // cubeManager.SetLampAll(color);
                    //     cubeManager.AddOrGetCube(address).SetMotor();
                    // }
                    break;

                case "battery":
                    byte value = m.Payload[0];
                    cubeManager.NotifyBattery(address, value);
                    break;
            }
        });
        return null;
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
        await client.DisconnectAsync();
        client.Dispose();
        client = null;
        // await server.Stop();
        Debug.LogWarning("Application Quit");
    }

}
