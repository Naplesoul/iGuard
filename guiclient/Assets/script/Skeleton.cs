using System.Collections;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Net;
using System.Text;
using System.Threading;
using UnityEngine;
using UnityEngine.UI;
using System.IO;
using System;

[System.Serializable]
public class KeyPack_Vector3{
    public float x;
    public float y;
    public float z;
}

[System.Serializable]
public class KeyPack_NodeInfo{
    public string pose;
    public bool danger;
    public KeyPack_Vector3[] nodes;
}


[System.Serializable]
public class WebPack_Vector3{
    public float x;
    public float y;
    public float z;
    public float score;
}

[System.Serializable]
public class WebPack_NodeInfo{
    public long frame_id;
    public WebPack_Vector3[] body_nodes;
    public WebPack_Vector3[] left_hand_nodes;
    public WebPack_Vector3[] right_hand_nodes;

}
public class Skeleton : MonoBehaviour
{
    private static string ip = "0.0.0.0";
    private static int port = 50002;
    private static Socket socket;

    private GameObject[] body_nodes;
    private GameObject[] body_bones;

    private GameObject[] lh_nodes;
    private GameObject[] lh_bones;
    private GameObject[] rh_nodes;
    private GameObject[] rh_bones;


    private Vector2Int[] bnMap, hbnMap;
    private Vector3[] body_nodePos, body_lastNodePos;
    private Vector3[] lh_nodePos, lh_lastNodePos;
    private Vector3[] rh_nodePos, rh_lastNodePos;

    private bool[] body_nodeActive, body_boneActive;
    private bool[] lh_nodeActive, lh_boneActive;
    private bool[] rh_nodeActive, rh_boneActive;
    public GameObject nodePrefab;
    public GameObject pPrefab;
    public GameObject bonePrefab;
    private Vector3[] keyPose;

    private GameObject[] preNodes;
    private Vector3[] prePose;

    private string[] keyPoseList;
    private int currKeyPoseIndex;
    private long last_frame_id;

    public Material keyMat;
    public Material defMat;
    public Material preMat;
    public Material dangerMat;
    public Text dangerText;
    public Text simiText;
    public Text nextText;
    public Text timeText;

    public GameObject lathe;
    public GameObject driller;
    private GameObject currentObject;

    // Start is called before the first frame update
    void Start()
    {
        body_nodes = new GameObject[19];
        lh_nodes = new GameObject[21];
        rh_nodes = new GameObject[21];
        body_bones = new GameObject[18];
        lh_bones = new GameObject[21];
        rh_bones = new GameObject[21];
        body_nodePos = new Vector3[19];
        body_lastNodePos = new Vector3[19];
        lh_nodePos = new Vector3[21];
        rh_nodePos = new Vector3[21];
        lh_lastNodePos = new Vector3[21];
        rh_lastNodePos = new Vector3[21];

        body_nodeActive = new bool[19];
        body_boneActive = new bool[18];
        lh_nodeActive = new bool[21];
        lh_boneActive = new bool[21];
        rh_nodeActive = new bool[21];
        rh_boneActive = new bool[21];

        bnMap = new Vector2Int[18];
        hbnMap = new Vector2Int[21];
        keyPose = new Vector3[19];
        keyPoseList = new string[0];
        prePose = new Vector3[19];
        preNodes = new GameObject[19];
        currKeyPoseIndex = 0;
        last_frame_id = -1;
        
        for (int i = 0; i < 19 ;i ++){
            body_nodes[i] = GameObject.Instantiate(nodePrefab, new Vector3(0, -10, 0), new Quaternion());
            body_nodes[i].name = "node-" + i;
            preNodes[i] = GameObject.Instantiate(pPrefab, new Vector3(0, -10, 0), new Quaternion());
            preNodes[i].name = "pNode-" + i;
        }
        for (int i = 0; i < 18 ;i ++){
            body_bones[i] = GameObject.Instantiate(bonePrefab, new Vector3(0, -10, 0), new Quaternion());
            body_bones[i].name = "bone-" + i;
        }
        for (int i = 0; i < 21 ;i ++){
            lh_nodes[i] = GameObject.Instantiate(nodePrefab, new Vector3(0, -10, 0), new Quaternion());
            lh_nodes[i].name = "lhnode-" + i;
            lh_nodes[i].transform.localScale = new Vector3(0.01f, 0.01f, 0.01f); 
        }
        for (int i = 0; i < 21 ;i ++){
            rh_nodes[i] = GameObject.Instantiate(nodePrefab, new Vector3(0, -10, 0), new Quaternion());
            rh_nodes[i].name = "rhnode-" + i;
            rh_nodes[i].transform.localScale = new Vector3(0.01f, 0.01f, 0.01f); 
        }
        for (int i = 0; i < 21 ;i ++){
            lh_bones[i] = GameObject.Instantiate(bonePrefab, new Vector3(0, -10, 0), new Quaternion());
            lh_bones[i].name = "bone-" + i;
        }
        for (int i = 0; i < 21 ;i ++){
            rh_bones[i] = GameObject.Instantiate(bonePrefab, new Vector3(0, -10, 0), new Quaternion());
            rh_bones[i].name = "bone-" + i;
        }
        bnMap[0] = new Vector2Int(0, 1);
        bnMap[1] = new Vector2Int(1, 4);
        bnMap[2] = new Vector2Int(4, 2);
        bnMap[3] = new Vector2Int(2, 3);
        bnMap[4] = new Vector2Int(4, 5);
        bnMap[5] = new Vector2Int(5, 6);
        bnMap[6] = new Vector2Int(6, 7);
        bnMap[7] = new Vector2Int(7, 8);
        bnMap[8] = new Vector2Int(4, 9);
        bnMap[9] = new Vector2Int(9, 10);
        bnMap[10] = new Vector2Int(10, 11);
        bnMap[11] = new Vector2Int(11, 12);
        bnMap[12] = new Vector2Int(3, 13);
        bnMap[13] = new Vector2Int(13, 14);
        bnMap[14] = new Vector2Int(14, 15);
        bnMap[15] = new Vector2Int(3, 16);
        bnMap[16] = new Vector2Int(16, 17);
        bnMap[17] = new Vector2Int(17, 18);

        hbnMap[0] = new Vector2Int(0, 1);
        hbnMap[1] = new Vector2Int(1, 2);
        hbnMap[2] = new Vector2Int(2, 3);
        hbnMap[3] = new Vector2Int(3, 4);
        hbnMap[4] = new Vector2Int(0, 5);
        hbnMap[5] = new Vector2Int(5, 6);
        hbnMap[6] = new Vector2Int(6, 7);
        hbnMap[7] = new Vector2Int(7, 8);
        hbnMap[8] = new Vector2Int(5, 9);
        hbnMap[9] = new Vector2Int(9, 10);
        hbnMap[10] = new Vector2Int(10, 11);
        hbnMap[11] = new Vector2Int(11, 12);
        hbnMap[12] = new Vector2Int(9, 13);
        hbnMap[13] = new Vector2Int(13, 14);
        hbnMap[14] = new Vector2Int(14, 15);
        hbnMap[15] = new Vector2Int(15, 16);
        hbnMap[16] = new Vector2Int(13, 17);
        hbnMap[17] = new Vector2Int(17, 18);
        hbnMap[18] = new Vector2Int(18, 19);
        hbnMap[19] = new Vector2Int(19, 20);
        hbnMap[20] = new Vector2Int(0, 17);

        CaseChanged(0);
        
        socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
        socket.Bind(new IPEndPoint(IPAddress.Parse(ip), port));
        Thread threadReceive = new Thread(new ThreadStart(receiveFromClient));
        threadReceive.Start();

        Debug.Log("skeleton server initialized successfully!");
    }

    // Update is called once per frame
    void Update()
    {
        for (int i = 0; i < 19 ;i ++){
            body_nodes[i].transform.localPosition = body_nodePos[i];
            body_nodes[i].SetActive(body_nodeActive[i]);
            preNodes[i].transform.localPosition = prePose[i];
        }
        if (!body_nodeActive[8]){
            body_nodes[7].transform.localScale = new Vector3(0.02f, 0.02f, 0.02f); 
        }
        if (!body_nodeActive[12]){
            body_nodes[11].transform.localScale = new Vector3(0.02f, 0.02f, 0.02f); 
        }
        for (int i = 0; i < 21 ;i ++){
            lh_nodes[i].transform.localPosition = lh_nodePos[i];
            lh_nodes[i].SetActive(lh_nodeActive[i]);
        }
        for (int i = 0; i < 21 ;i ++){
            rh_nodes[i].transform.localPosition = rh_nodePos[i];
            rh_nodes[i].SetActive(rh_nodeActive[i]);
        }
        for (int i = 0; i < 18 ;i ++){
            Vector3 a = body_nodePos[bnMap[i].x], b = body_nodePos[bnMap[i].y];
            Vector3 forward = new Vector3(1, 1, 1);
            Vector3 upward = b - a;
            if (upward.x != 0){
                forward.x = -(upward.y + upward.z)/upward.x;
            }else if (upward.y != 0){
                forward.y = -(upward.x + upward.z)/upward.y;
            }else if (upward.z != 0){
                forward.z = -(upward.y + upward.x)/upward.z;
            }
            body_bones[i].SetActive(body_boneActive[i]);
            body_bones[i].transform.localPosition = (a + b) / 2;
            body_bones[i].transform.localRotation = Quaternion.LookRotation(forward, upward);
            body_bones[i].transform.localScale = new Vector3(0.02f, upward.magnitude / 2, 0.02f); 
        }
        for (int i = 0; i < 21 ;i ++){
            Vector3 a = lh_nodePos[hbnMap[i].x], b = lh_nodePos[hbnMap[i].y];
            Vector3 forward = new Vector3(1, 1, 1);
            Vector3 upward = b - a;
            if (upward.x != 0){
                forward.x = -(upward.y + upward.z)/upward.x;
            }else if (upward.y != 0){
                forward.y = -(upward.x + upward.z)/upward.y;
            }else if (upward.z != 0){
                forward.z = -(upward.y + upward.x)/upward.z;
            }
            lh_bones[i].SetActive(lh_boneActive[i]);
            lh_bones[i].transform.localPosition = (a + b) / 2;
            lh_bones[i].transform.localRotation = Quaternion.LookRotation(forward, upward);
            lh_bones[i].transform.localScale = new Vector3(0.005f, upward.magnitude / 2, 0.005f); 
        }
        for (int i = 0; i < 21 ;i ++){
            Vector3 a = rh_nodePos[hbnMap[i].x], b = rh_nodePos[hbnMap[i].y];
            Vector3 forward = new Vector3(1, 1, 1);
            Vector3 upward = b - a;
            if (upward.x != 0){
                forward.x = -(upward.y + upward.z)/upward.x;
            }else if (upward.y != 0){
                forward.y = -(upward.x + upward.z)/upward.y;
            }else if (upward.z != 0){
                forward.z = -(upward.y + upward.x)/upward.z;
            }
            rh_bones[i].SetActive(rh_boneActive[i]);
            rh_bones[i].transform.localPosition = (a + b) / 2;
            rh_bones[i].transform.localRotation = Quaternion.LookRotation(forward, upward);
            rh_bones[i].transform.localScale = new Vector3(0.005f, upward.magnitude / 2, 0.005f); 
        }

        float cos = GetSimilarity(keyPose, false);
        simiText.text = "当前步骤完成度：" + Mathf.Round(cos * 10000) / 100 + "%";
        if (cos > 0.9){
            body_nodes[0].GetComponent<MeshRenderer>().material = keyMat;
            NextKeyPos();
        }else{
            body_nodes[0].GetComponent<MeshRenderer>().material = defMat;
        }

        Camera.main.transform.localPosition = new Vector3(body_nodePos[3].x - 1.8f, body_nodePos[3].y + 1.2f, body_nodePos[3].z - 0.8f);
        timeText.text = string.Format("{0:T}", DateTime.Now);
    }

    private float GetSimilarity(Vector3[] kp, bool strict){
        //Vector3 m = new Vector3();
        float r = 0;
        // if (!strict){
        //     for (int i = 0; i < 19 ;i ++){
        //         m += keyPose[i] - nodePos[i];
        //     }
        //     m = m / 20;
        // }
        float m1 = 0, m2 = 0;
        // for (int i = 0; i < 25 ;i ++){
        //     if (nodes[i] == null) continue;
        //     r += (nodePos[i] + m).x * (kp[i].x);
        //     r += (nodePos[i] + m).y * (kp[i].y);
        //     r += (nodePos[i] + m).z * (kp[i].z);
        //     m1 += (nodePos[i] + m).magnitude * (nodePos[i] + m).magnitude;
        //     m2 += kp[i].magnitude * kp[i].magnitude;
        // }
        for (int i = 0; i < 11; i ++){
            if (i == 3 || i == 7) continue;
            Vector3 a = body_nodePos[bnMap[i].x], b = body_nodePos[bnMap[i].y];
            Vector3 ak = kp[bnMap[i].x], bk = kp[bnMap[i].y];
            Vector3 upward = (b - a), upwardk = (bk - ak).normalized * (b - a).magnitude;
            r += upward.x * upwardk.x;
            r += upward.y * upwardk.y;
            r += upward.z * upwardk.z;
            m1 += upward.magnitude * upward.magnitude;
            m2 += upwardk.magnitude * upwardk.magnitude;
        }
        float cos = r / Mathf.Sqrt(m1) / Mathf.Sqrt(m2);
        return cos * cos;
    }

    private void NextKeyPos(){
        currKeyPoseIndex ++;
        if (currKeyPoseIndex < keyPoseList.Length){
            LoadKeyPos(keyPoseList[currKeyPoseIndex]);
        }else {
            nextText.text = "无关键步骤信息";
            foreach (Transform child in currentObject.transform)
            {
                if(child.gameObject.name.Contains("danger")){
                    CapsuleCollider collider = child.gameObject.GetComponent<CapsuleCollider>();
                    collider.enabled = false;
                    child.gameObject.GetComponent<MeshRenderer>().material = preMat;
                    break;
                }
            }
        }
    }

    private void LoadKeyPos(string filename){
        Debug.Log("loading key pose " + filename);
        StreamReader sr = new StreamReader("./Assets/Pose/" + filename + ".json", Encoding.UTF8);
        string content =  sr.ReadToEnd();
        sr.Close();
        KeyPack_NodeInfo node_info = JsonUtility.FromJson<KeyPack_NodeInfo>(content);
        for (int i = 0; i < 19 ;i ++){
            keyPose[i].x = node_info.nodes[i].x / 1000;
            keyPose[i].y = node_info.nodes[i].y / 1000;
            keyPose[i].z = node_info.nodes[i].z / 1000;
        }
        foreach (Transform child in currentObject.transform)
        {
            if(child.gameObject.name.Contains("danger")){
                CapsuleCollider collider = child.gameObject.GetComponent<CapsuleCollider>();
                if (node_info.danger){
                    collider.enabled = true;
                    child.gameObject.GetComponent<MeshRenderer>().material = dangerMat;
                }else{
                    collider.enabled = false;
                    child.gameObject.GetComponent<MeshRenderer>().material = preMat;
                }
                break;
            }
        }
        nextText.text = "请执行关键步骤("+ (currKeyPoseIndex + 1) +"/"+ keyPoseList.Length +")：" + node_info.pose;
    }

    private void SetNodePos(WebPack_NodeInfo ni){

        if (last_frame_id > ni.frame_id) return;

        if (last_frame_id == ni.frame_id){

        }else{
            for (int i = 0; i < 19 ;i ++){
                body_lastNodePos[i] = body_nodePos[i];
                body_nodeActive[i] = (ni.body_nodes[i].score != 0);
                //body_nodeActive[i] = true;
                body_nodePos[i].x = body_nodePos[i].x * (1 - ni.body_nodes[i].score / 100) + (ni.body_nodes[i].x / 1000 * ni.body_nodes[i].score / 100);
                body_nodePos[i].y = body_nodePos[i].y * (1 - ni.body_nodes[i].score / 100) + (ni.body_nodes[i].y / 1000 * ni.body_nodes[i].score / 100);
                body_nodePos[i].z = body_nodePos[i].z * (1 - ni.body_nodes[i].score / 100) + (ni.body_nodes[i].z / 1000 * ni.body_nodes[i].score / 100);
            }
            for (int i = 0; i < 18 ;i ++){
                body_boneActive[i] = (ni.body_nodes[bnMap[i].x].score != 0 && ni.body_nodes[bnMap[i].y].score != 0);
                //body_boneActive[i] = true;
            }
            if (ni.left_hand_nodes != null){
                body_nodeActive[8] = false;
                body_boneActive[7] = false;
                for (int i = 0; i < 21;i ++){
                    lh_lastNodePos[i] = lh_nodePos[i];
                    lh_nodeActive[i] = (ni.left_hand_nodes[i].score != 0);
                    lh_nodePos[i].x = lh_nodePos[i].x * (1 - ni.left_hand_nodes[i].score / 100) + (ni.left_hand_nodes[i].x / 1000 * ni.left_hand_nodes[i].score / 100);
                    lh_nodePos[i].y = lh_nodePos[i].y * (1 - ni.left_hand_nodes[i].score / 100) + (ni.left_hand_nodes[i].y / 1000 * ni.left_hand_nodes[i].score / 100);
                    lh_nodePos[i].z = lh_nodePos[i].z * (1 - ni.left_hand_nodes[i].score / 100) + (ni.left_hand_nodes[i].z / 1000 * ni.left_hand_nodes[i].score / 100);
                }
                for (int i = 0; i < 21 ;i ++){
                    lh_boneActive[i] = (ni.left_hand_nodes[hbnMap[i].x].score != 0 && ni.left_hand_nodes[hbnMap[i].y].score != 0);
                }
            }
            if (ni.right_hand_nodes != null){
                body_nodeActive[12] = false;
                body_boneActive[11] = false;
                for (int i = 0; i < 21 ;i ++){
                    rh_lastNodePos[i] = rh_nodePos[i];
                    rh_nodeActive[i] = (ni.right_hand_nodes[i].score != 0);
                    rh_nodePos[i].x = rh_nodePos[i].x * (1 - ni.right_hand_nodes[i].score / 100) + (ni.right_hand_nodes[i].x / 1000 * ni.right_hand_nodes[i].score / 100);
                    rh_nodePos[i].y = rh_nodePos[i].y * (1 - ni.right_hand_nodes[i].score / 100) + (ni.right_hand_nodes[i].y / 1000 * ni.right_hand_nodes[i].score / 100);
                    rh_nodePos[i].z = rh_nodePos[i].z * (1 - ni.right_hand_nodes[i].score / 100) + (ni.right_hand_nodes[i].z / 1000 * ni.right_hand_nodes[i].score / 100);
                }
                for (int i = 0; i < 21 ;i ++){
                    rh_boneActive[i] = (ni.right_hand_nodes[hbnMap[i].x].score != 0 && ni.right_hand_nodes[hbnMap[i].y].score != 0);
                }
            }
            if (last_frame_id != -1){
                for (int i = 0; i < 19 ;i ++){
                    prePose[i].x = (body_nodePos[i].x - body_lastNodePos[i].x) < 0.1 ? (2 * body_nodePos[i].x - body_lastNodePos[i].x) : body_lastNodePos[i].x;
                    prePose[i].y = (body_nodePos[i].y - body_lastNodePos[i].y) < 0.1 ? (2 * body_nodePos[i].y - body_lastNodePos[i].y) : body_lastNodePos[i].y;
                    prePose[i].z = (body_nodePos[i].z - body_lastNodePos[i].z) < 0.1 ? (2 * body_nodePos[i].z - body_lastNodePos[i].z) : body_lastNodePos[i].z;
                }
            }
            last_frame_id = ni.frame_id;
        }
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
            string node_info_str = Encoding.UTF8.GetString(data, 0, recv);
            Debug.Log(node_info_str);

            WebPack_NodeInfo node_info = JsonUtility.FromJson<WebPack_NodeInfo>(node_info_str);
            //Debug.Log(node_info.node_num);
            //Debug.Log(node_info.nodes);
            SetNodePos(node_info);
        }
    }
    public void CaseChanged(int value){
        Debug.Log("Change to mode " + value);
        lathe.SetActive(value == 0);
        driller.SetActive(value == 1);
        if (value == 0){
            currentObject = lathe;
            keyPoseList = new string[5]{"lathe/tighten", "lathe/start", "lathe/cut", "lathe/stop", "lathe/release"};
            currKeyPoseIndex = -1;
            NextKeyPos();
        }else if (value == 1){
            currentObject = driller;
            keyPoseList = new string[5]{"driller/tighten", "driller/start", "driller/drill", "driller/stop", "driller/release"};
            currKeyPoseIndex = -1;
            NextKeyPos();
        }
    }

    public void DangerModeChanged(bool f){
        dangerText.enabled = f;
        for (int i = 0; i < 19 ;i ++){
            body_nodes[i].GetComponent<SphereCollider>().enabled = f;
        }
    }

    public void KeyModeChanged(bool f){
        simiText.enabled = f;
        nextText.enabled = f;
        if (f){
            currKeyPoseIndex = -1;
            NextKeyPos();
        }
    }

}
