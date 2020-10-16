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
    private HashSet<string> availableBridegs;
    // private HashSet<string> connectingCubes;

    private Server server;
    private IMqttClientOptions options;
    private IMqttClient client;


    async void Start()
    {
        Application.targetFrameRate = 60;

        statusText.text = "Starting...";

        server = new Server();
        await server.Start();

        client = new MqttFactory().CreateMqttClient();
        client.UseConnectedHandler(this);
        client.UseDisconnectedHandler(this);
        client.UseApplicationMessageReceivedHandler(this);

        cubeManager = GetComponent<CubeManager>();
        cubeManager.Publisher = client;

        availableBridegs = new HashSet<string>();
        // connectingCubes = new HashSet<string>();

        options = new MqttClientOptionsBuilder()
            .WithClientId("controller")
            .WithTcpServer("localhost")
            .WithCleanSession()
            .Build();
        await client.ConnectAsync(options);
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
                    // if (connectingCubes.Contains(payload))
                    // {
                    //     return;
                    // }
                    var bridge = availableBridegs.FirstOrDefault();
                    logger.ZLogDebug(bridge);
                    if (!string.IsNullOrEmpty(bridge))
                    {
                        availableBridegs.Remove(bridge);
                        var message = new MqttApplicationMessageBuilder()
                            .WithTopic($"{bridge}/newcube")
                            .WithPayload(m.Payload)
                            .WithAtLeastOnceQoS()
                            .Build();
                        client.PublishAsync(message, CancellationToken.None);
                        // connectingCubes.Add(payload);
                    }
                    return;
            }
            var t = m.Topic.Split('/');
            var address = t[0];
            switch (t[1])
            {
                case "available":
                    logger.ZLogTrace("available cliente: {0}", address);
                    availableBridegs.Add(address);
                    break;

                case "connected":
                    // connectingCubes.Remove(address);
                    var cube = cubeManager.AddOrGetCube(address);
                    cube.SetLamp(Color.white);
                    break;

                case "disconnected":
                    // connectingCubes.Remove(address);
                    break;

                case "position":
                    cubeManager.SetPosition(address, m.Payload);
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
                    cubeManager.SetBattery(address, value);
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

        if (Input.GetKeyDown(KeyCode.Space))
        {
            RandomRotate();
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
        cubeManager.SetLampAll(Colors[count++ % Colors.Length]);
    }


    int angle = 0;

    public void RandomRotate()
    {
        cubeManager.SetMotorAll(angle);
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
        await server.Stop();
    }

}
