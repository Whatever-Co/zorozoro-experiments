using System;
using System.Net;
using System.Net.Sockets;
using System.Threading.Tasks;
using UnityEngine;


public class TcpServer : MonoBehaviour
{

    public string ipAddress = "0.0.0.0";
    public int port = 11111;

    public event Action<Bridge> Connected;

    private TcpListener listener;


    public void Start()
    {
        var ip = IPAddress.Parse(ipAddress);
        listener = new TcpListener(ip, port);
        listener.Start();
        Task.Run(() =>
        {
            while (listener != null)
            {
                var client = listener.AcceptTcpClient();
                print("TcpServer: Accepted " + client.Client.RemoteEndPoint);
                var bridge = new Bridge(client);
                Connected.Invoke(bridge);
            }
        });
    }


    private void OnApplicationQuit()
    {
        if (listener != null)
        {
            listener.Stop();
            listener = null;
        }
    }

}
