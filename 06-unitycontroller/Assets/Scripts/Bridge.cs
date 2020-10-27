using Microsoft.Extensions.Logging;
using System.Collections.Generic;
using System.Linq;
using System.IO;
using System.Text;
using System.Threading;
using UnityEngine;
using ZLogger;


public class Bridge
{

    static readonly ILogger<Bridge> logger = LogManager.GetLogger<Bridge>();


    public string Address { get; private set; }
    public int AvailableSlot { get; set; } = 0;


    public Bridge(string address)
    {
        Address = address;
    }


    public override string ToString()
    {
        return $"[Bridge Address={Address}, AvailableSlot={AvailableSlot}]";
    }

}
