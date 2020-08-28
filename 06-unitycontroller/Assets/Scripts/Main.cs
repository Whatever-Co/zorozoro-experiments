using Microsoft.Extensions.Logging;
using MQTTnet;
using MQTTnet.Protocol;
using MQTTnet.Server;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using UnityEngine;
using UnityEngine.UI;
using ZLogger;


public class Main : MonoBehaviour
{

    private static readonly ILogger<Main> logger = LogManager.GetLogger<Main>();


    public LayoutGroup InfoGroup;
    public GameObject BridgeInfoPrefab;

    private IMqttServer server;
    private Dictionary<string, Bridge> bridges = new Dictionary<string, Bridge>();
    private CubeManager cubeManager;


    void Start()
    {
        logger.ZLogDebug("Init!");

        foreach (Transform t in InfoGroup.transform)
        {
            Destroy(t.gameObject);
        }

        cubeManager = new CubeManager();

        var optionsBuilder = new MqttServerOptionsBuilder()
        .WithDefaultEndpoint()
        .WithConnectionValidator(ConnectionValidator)
        .WithSubscriptionInterceptor(SubscriptionInterceptor);
        // .WithApplicationMessageInterceptor(ApplicationMessageInterceptor);

        server = new MqttFactory().CreateMqttServer();
        server.UseClientDisconnectedHandler(ClientDisconnectedHandler);
        server.UseApplicationMessageReceivedHandler(ApplicationMessageReceivedHandler);
        server.StartAsync(optionsBuilder.Build());
    }


    void ConnectionValidator(MqttConnectionValidatorContext c)
    {
        c.ReasonCode = MqttConnectReasonCode.Success;
        LogMessage(c, false);
    }


    void SubscriptionInterceptor(MqttSubscriptionInterceptorContext c)
    {
        c.AcceptSubscription = true;
        LogMessage(c, true);
    }


    void ApplicationMessageInterceptor(MqttApplicationMessageInterceptorContext c)
    {
        c.AcceptPublish = true;
        // LogMessage(c);
    }


    void ClientDisconnectedHandler(MqttServerClientDisconnectedEventArgs e)
    {
        logger.ZLogWarning($"Client disconnected: id={e.ClientId}, type={e.DisconnectType}");
        // if (bridges.TryGetValue(e.ClientId, out var bridge))
        // {
        //     bridges.Remove(e.ClientId);
        //     Dispatcher.runOnUiThread(() =>
        //     {
        //         Object.Destroy(bridge.InfoPanel.gameObject);
        //         bridge.InfoPanel = null;
        //         bridge.Dispose();
        //     });
        // }
    }


    void ApplicationMessageReceivedHandler(MqttApplicationMessageReceivedEventArgs e)
    {
        try
        {
            if (string.IsNullOrEmpty(e.ClientId))
            {
                return;
            }
            var m = e.ApplicationMessage;
            var payload = Encoding.ASCII.GetString(m.Payload);
            logger.ZLogDebug($"Message received: ClientId = {e.ClientId}, Topic = {m.Topic}, Payload = {payload}");
            switch (m.Topic)
            {
                case "hello":
                    HelloHandler(e.ClientId, payload);
                    break;
                case "newcube":
                    NewCubeHandler(payload);
                    break;
                default:
                    if (bridges.TryGetValue(e.ClientId, out var bridge))
                    {
                        bridge.ProcessPayload(m.Topic, payload);
                    }
                    break;
            }
        }
        catch (System.Exception ex)
        {
            logger.ZLogError(ex, "what..?");
        }
    }


    void HelloHandler(string clientId, string mode)
    {
        switch (mode)
        {
            case "scanner":
                // do nothing...
                break;
            case "bridge":
                if (!bridges.ContainsKey(clientId))
                {
                    var b = new Bridge(clientId, server);
                    bridges.Add(clientId, b);
                    Dispatcher.runOnUiThread(() =>
                    {
                        var info = Instantiate(BridgeInfoPrefab).GetComponent<BridgeInfo>();
                        logger.ZLogDebug(info.ToString());
                        info.transform.SetParent(InfoGroup.transform);
                        b.InfoPanel = info;
                    });
                }
                break;
            default:
                logger.ZLogWarning("Unknow mode: " + mode);
                break;
        }
    }


    void NewCubeHandler(string address)
    {
        var kv = bridges
        .Where(i => !i.Value.IsBusy && i.Value.NumConnectedCubes < Bridge.MAX_CUBES_PER_BRIDGE)
        .OrderBy(i => i.Value.NumConnectedCubes)
        .FirstOrDefault();
        if (kv.Value != null)
        {
            kv.Value.Connect(cubeManager.CreateCube(address));
        }
        else
        {
            logger.ZLogDebug("No bridges available now...");
        }
    }


    void Update()
    {
        if (Input.GetKeyDown(KeyCode.Space))
        {
            var color = new Color(Random.value, Random.value, Random.value);
            logger.ZLogDebug(color.ToString());
            cubeManager.SetLamp(color);
        }
    }


    void OnApplicationQuit()
    {
        server?.StopAsync();
    }


    private static void LogMessage(MqttConnectionValidatorContext context, bool showPassword)
    {
        if (context == null)
        {
            return;
        }

        if (showPassword)
        {
            logger.ZLogDebug(
                $"New connection: ClientId = {context.ClientId}, Endpoint = {context.Endpoint},"
                + $" Username = {context.Username}, Password = {context.Password},"
                + $" CleanSession = {context.CleanSession}");
        }
        else
        {
            logger.ZLogDebug(
                $"New connection: ClientId = {context.ClientId}, Endpoint = {context.Endpoint},"
                + $" Username = {context.Username}, CleanSession = {context.CleanSession}");
        }
    }


    private static void LogMessage(MqttSubscriptionInterceptorContext context, bool successful)
    {
        if (context == null)
        {
            return;
        }

        logger.ZLogDebug(successful ? $"New subscription: ClientId = {context.ClientId}, TopicFilter = {context.TopicFilter}" : $"Subscription failed for clientId = {context.ClientId}, TopicFilter = {context.TopicFilter}");
    }


    private static void LogMessage(MqttApplicationMessageInterceptorContext context)
    {
        if (context == null)
        {
            return;
        }

        var payload = context.ApplicationMessage?.Payload == null ? null : Encoding.UTF8.GetString(context.ApplicationMessage?.Payload);

        logger.ZLogDebug(
            $"Message: ClientId = {context.ClientId}, Topic = {context.ApplicationMessage?.Topic},"
            + $" Payload = {payload}, QoS = {context.ApplicationMessage?.QualityOfServiceLevel},"
            + $" Retain-Flag = {context.ApplicationMessage?.Retain}");
    }

}
