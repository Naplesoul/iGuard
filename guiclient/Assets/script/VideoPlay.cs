using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using System.Net.Sockets;
using System.Net;
using System.Threading;
using System.Text;

public class VideoPlay : MonoBehaviour
{
    private static string ip = "0.0.0.0";
    private static int port = 50004;

    private int pack_size = 640 * 360 * 3 * 10;
    private static Socket socket;

    private byte[] image_data;

    public RawImage image;
    private Texture2D texture;

    // Start is called before the first frame update
    void Start()
    {
        socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
        socket.Bind(new IPEndPoint(IPAddress.Parse(ip), port));
        Thread threadReceive = new Thread(new ThreadStart(receiveFromClient));
        threadReceive.Start();

        Debug.Log("similarity server initialized successfully!");

        texture = new Texture2D(640, 360);
        image_data = new byte[pack_size];
    }

    // Update is called once per frame
    void Update()
    {
        if (image_data != null){
            texture.LoadImage(image_data);
            image.texture = texture;
        }
    }

    void receiveFromClient(){
        int recv;
        byte[] data;
        IPEndPoint sender = new IPEndPoint(IPAddress.Any, 0);
        EndPoint Remote = (EndPoint)(sender);
        while(true){
            data = new byte[pack_size];
            recv = socket.ReceiveFrom(data, ref Remote);
            Debug.Log(recv.ToString() + " bytes received from " + Remote.ToString() + ":");
            //string node_info_str = Encoding.UTF8.GetString(data, 0, recv);
            //Debug.Log(node_info_str);
            // int p = 0;
            // for (int i = 0; i < 360; i++){
            //     for (int j = 0; j < 640; j++){
            //         texture.SetPixel(i, j, new Color(data[p], data[p+1], data[p+2]));
            //         p += 3;
            //     }
            // }
            image_data = data;
        }
    }
}
