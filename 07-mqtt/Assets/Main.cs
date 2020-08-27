using MQTTnet;
using MQTTnet.Protocol;
using MQTTnet.Server;
using System.Text;
using UnityEngine;


public class Main : MonoBehaviour
{

    IMqttServer server;


    void Start()
    {
        var optionsBuilder = new MqttServerOptionsBuilder()
        .WithDefaultEndpoint()
        .WithDefaultEndpointPort(1883)
        .WithConnectionValidator(c =>
        {
            c.ReasonCode = MqttConnectReasonCode.Success;
            LogMessage(c, false);
        })
        .WithSubscriptionInterceptor(c =>
        {
            c.AcceptSubscription = true;
            LogMessage(c, true);
        })
        .WithApplicationMessageInterceptor(c =>
        {
            c.AcceptPublish = true;
            LogMessage(c);
        });

        server = new MqttFactory().CreateMqttServer();
        server.StartAsync(optionsBuilder.Build());
    }


    void OnApplicationQuit()
    {
        server.StopAsync();
    }


    private static void LogMessage(MqttConnectionValidatorContext context, bool showPassword)
    {
        if (context == null)
        {
            return;
        }

        if (showPassword)
        {
            Debug.Log(
                $"New connection: ClientId = {context.ClientId}, Endpoint = {context.Endpoint},"
                + $" Username = {context.Username}, Password = {context.Password},"
                + $" CleanSession = {context.CleanSession}");
        }
        else
        {
            Debug.Log(
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

        Debug.Log(successful ? $"New subscription: ClientId = {context.ClientId}, TopicFilter = {context.TopicFilter}" : $"Subscription failed for clientId = {context.ClientId}, TopicFilter = {context.TopicFilter}");
    }


    private static void LogMessage(MqttApplicationMessageInterceptorContext context)
    {
        if (context == null)
        {
            return;
        }

        var payload = context.ApplicationMessage?.Payload == null ? null : Encoding.UTF8.GetString(context.ApplicationMessage?.Payload);

        Debug.Log(
            $"Message: ClientId = {context.ClientId}, Topic = {context.ApplicationMessage?.Topic},"
            + $" Payload = {payload}, QoS = {context.ApplicationMessage?.QualityOfServiceLevel},"
            + $" Retain-Flag = {context.ApplicationMessage?.Retain}");
    }

}
