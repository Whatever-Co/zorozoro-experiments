using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Threading.Tasks;
using UnityEngine;
using ZLogger;


public class BridgeManager : MonoBehaviour
{

    private static readonly ILogger<BridgeManager> logger = LogManager.GetLogger<BridgeManager>();


    private readonly IPAddress SERVER_ADDRESS = IPAddress.Any;
    private readonly int SERVER_PORT = 11111;

    public event Action<string, string, string, byte[]> OnMessage;

    private TcpListener listener;
    private Dictionary<string, Bridge> bridges = new Dictionary<string, Bridge>();

    public int BridgeCount { get => bridges.Count(); }


    public void Start()
    {
        listener = new TcpListener(SERVER_ADDRESS, SERVER_PORT);
        listener.Start();
        Task.Run(() =>
        {
            while (listener != null)
            {
                var client = listener.AcceptTcpClient();
                var endPoint = client.Client.RemoteEndPoint as IPEndPoint;
                var address = endPoint.Address.ToString();
                logger.ZLogDebug("Bridge connection accepted: {0}", endPoint);

                Bridge bridge;
                if (bridges.ContainsKey(address))
                {
                    bridge = bridges[address];
                    bridge.OnMessage -= HandleMessage;
                    bridges.Remove(address);
                    bridge.Stop();
                }

                bridge = new Bridge(client);
                bridge.OnMessage += HandleMessage;
                bridge.Start();
                bridges[address] = bridge;
            }
        });
    }


    public void ConnectToCube(string address)
    {
        var kv = bridges.Where(x => !x.Value.ConnectingCube).OrderByDescending(x => x.Value.AvailableSlot).FirstOrDefault();
        // logger.ZLogWarning(kv);
        kv.Value?.ConnectToCube(address);
    }


    public void SendMotorCommand(string bridgeAddress, string cubeAddress, byte[] payload)
    {
        if (!bridges.ContainsKey(bridgeAddress))
        {
            logger.ZLogWarning($"SendMotorCommand: No bridge found with address {bridgeAddress} for {cubeAddress}");
            return;
        }
        bridges[bridgeAddress].SendMotorCommand(cubeAddress, payload);
    }


    public void SendLampCommand(string bridgeAddress, string cubeAddress, byte[] payload)
    {
        if (!bridges.ContainsKey(bridgeAddress))
        {
            logger.ZLogWarning($"SendLampCommand: No bridge found with address {bridgeAddress} for {cubeAddress}");
            return;
        }
        bridges[bridgeAddress].SendLampCommand(cubeAddress, payload);
    }


    private void HandleMessage(Bridge bridge, string cubeAddress, string command, byte[] payload)
    {
        OnMessage(bridge.Address, cubeAddress, command, payload);
    }


    private void OnApplicationQuit()
    {
        if (listener != null)
        {
            listener.Stop();
            listener = null;
        }
        foreach (var kv in bridges)
        {
            kv.Value.Stop();
        }
    }

}
