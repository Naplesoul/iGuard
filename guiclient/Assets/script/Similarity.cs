using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Net.Sockets;
using System.Net;
using System.Threading;
using System.Text;
using System;
using UnityEngine.UI;



[System.Serializable]
public class WebPack_Similarity{
    public long frame_id;
    public float value;
}

public class Similarity : MonoBehaviour
{
    private static string ip = "0.0.0.0";
    private static int port = 50003;


    private static Socket socket;

    private string act;
    private float simi;

    // Start is called before the first frame update
    void Start()
    {
        socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
        socket.Bind(new IPEndPoint(IPAddress.Parse(ip), port));
        Thread threadReceive = new Thread(new ThreadStart(receiveFromClient));
        threadReceive.Start();

        Debug.Log("similarity server initialized successfully!");
    }

    // Update is called once per frame
    void Update()
    {
        if (Skeleton.sim != simi) Skeleton.sim = simi;
    }

    void receiveFromClient(){
        int recv;
        byte[] data;
        IPEndPoint sender = new IPEndPoint(IPAddress.Any, 0);
        EndPoint Remote = (EndPoint)(sender);
        while(true){
            data = new byte[10240];
            recv = socket.ReceiveFrom(data, ref Remote);
            //Debug.Log(recv.ToString() + " bytes received from " + Remote.ToString() + ":");
            string info_str = Encoding.UTF8.GetString(data, 0, recv);
            Debug.Log(info_str);

            WebPack_Similarity info = JsonUtility.FromJson<WebPack_Similarity>(info_str);

            simi = info.value;
        }
    }
}
