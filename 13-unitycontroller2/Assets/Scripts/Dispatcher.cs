using System;
using System.Collections.Concurrent;
using UnityEngine;


public class Dispatcher : MonoBehaviour
{

    static ConcurrentQueue<Action> queue = new ConcurrentQueue<Action>();


    public static void runOnUiThread(Action action)
    {
        queue.Enqueue(action);
    }


    void Update()
    {
        while (queue.TryDequeue(out var action))
        {
            try
            {
                action?.Invoke();
            }
            catch (Exception e)
            {
                Debug.LogException(e);
            }
        }
    }

}
