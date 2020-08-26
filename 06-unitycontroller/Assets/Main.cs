using System;
using System.Net;
using System.Net.Sockets;
using System.Collections.Generic;
using UnityEngine;


public class Main : MonoBehaviour
{

    TcpListener listener;
    Client scanner;
    List<Client> bridges = new List<Client>();


    void Start()
    {
        var address = IPAddress.Parse("0.0.0.0");
        listener = new TcpListener(address, 12322);
        listener.Start();
        listener.BeginAcceptTcpClient(AcceptTcpClient, null);
    }


    void AcceptTcpClient(IAsyncResult result)
    {
        var client = new Client(listener.EndAcceptTcpClient(result));

        listener.BeginAcceptTcpClient(AcceptTcpClient, null);

        try
        {
            client.OnHello += OnHello;
            client.Start();
        }
        catch (Exception e)
        {
            Debug.LogException(e);
        }
    }


    private void OnHello(Client client, Client.Mode mode)
    {
        Debug.Log($"OnHello: {client.Address} is {mode}");
        switch (mode)
        {
            case Client.Mode.Scanner:
                scanner = client;
                client.OnNewCube += OnNewCube;
                break;
            case Client.Mode.Bridge:
                bridges.Add(client);
                break;
        }
    }


    private void OnNewCube(Client client, string address)
    {
        Debug.Log($"OnNewCube: {address}");
    }


    void OnApplicationQuit()
    {
        listener.Stop();
        scanner?.Stop();
        foreach (var bridge in bridges)
        {
            bridge.Stop();
        }
    }

}
