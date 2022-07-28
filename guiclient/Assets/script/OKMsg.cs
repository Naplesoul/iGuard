using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class OKMsg : MonoBehaviour
{
    // Start is called before the first frame update
    public Text okText;
    public GameObject okPanel;

    private static ArrayList msg_list;
    private static object lock_obj = new object();
    private static float last_msg_time;

    private class Message{
        public string text;
        public int id;
        private static int next_id = 0;
        public Message(string t){
            text = t; id = next_id;
            next_id ++;
        }
    } 

    void Start()
    {
        okPanel.SetActive(false);
        okText.text = "";
        msg_list = new ArrayList();
        last_msg_time = Time.time;
    }

    // Update is called once per frame
    void Update()
    {
        if (Time.time - last_msg_time < 3){
            return;
        }
        last_msg_time = Time.time;
        lock(lock_obj){
            if (msg_list.Count > 0){
                okPanel.SetActive(true);
                Message msg = (Message)msg_list[0];
                okText.text = msg.text;
                msg_list.RemoveAt(0);
            }else{
                okPanel.SetActive(false);
                okText.text = "";
            }
        }
    }
    public static int addAlertMsg(string msg){
        Message m;
        lock(lock_obj){
            m = new Message(msg);
            msg_list.Add(m);
        }
        return m.id;
    }
}
