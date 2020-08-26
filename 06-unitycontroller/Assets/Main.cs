using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
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
            Debug.LogWarning(e);
        }

        if (client == scanner)
        {
            scanner = null;
        }
        else
        {
            bridges.Remove(client);
        }
        Debug.Log($"Disconnected: {client}");
    }


    private void OnHello(Client client, Client.Mode mode)
    {
        Debug.Log($"OnHello: {client.Address} is {mode}");
        switch (mode)
        {
            case Client.Mode.Scanner:
                scanner = client;
                scanner.OnCubeFound += OnCubeFound;
                break;
            case Client.Mode.Bridge:
                bridges.Add(client);
                break;
        }
        client.OnHello -= OnHello;
    }


    private void OnCubeFound(Client client, Cube cube)
    {
        Debug.Log($"OnCubeFound: {cube}");
        var bridge = bridges.Where(b => !b.IsBusy && b.NumConnectedCubes < Client.MAX_CUBES_PER_CLIENT).OrderBy(b => b.NumConnectedCubes).FirstOrDefault();
        if (bridge == null)
        {
            Debug.LogWarning("no bridges available now...");
            return;
        }
        bridge.Connect(cube);
    }


    void OnApplicationQuit()
    {
        listener?.Stop();
        scanner?.Stop();
        foreach (var bridge in bridges)
        {
            bridge.Stop();
        }
    }

}
