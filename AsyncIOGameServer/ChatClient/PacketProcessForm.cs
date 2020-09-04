using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace csharp_test_client
{
    public partial class mainForm
    {
        void AddEchoMessageList(string message)
        {
            if (chatListBox.Items.Count > 512)
            {
                chatListBox.Items.Clear();
            }

            chatListBox.Items.Add(message);
            chatListBox.SelectedIndex = chatListBox.Items.Count - 1;
        }
    }
}
