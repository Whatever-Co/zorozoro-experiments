using Microsoft.Extensions.Logging;
using MQTTnet;
using MQTTnet.Protocol;
using MQTTnet.Server;
using System.Text;
using UnityEngine;
using ZLogger;
using System.Threading.Tasks;


public class Server
{

    private static readonly ILogger<Server> logger = LogManager.GetLogger<Server>();


    IMqttServer server;


    public Task Start()
    {
        var options = new MqttServerOptionsBuilder()
            .WithDefaultEndpoint()
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
        return server.StartAsync(options.Build());
    }


    public Task Stop()
    {
        return server?.StopAsync();
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

}
