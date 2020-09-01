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
            if (echoListBox.Items.Count > 512)
            {
                echoListBox.Items.Clear();
            }

            echoListBox.Items.Add(message);
            echoListBox.SelectedIndex = echoListBox.Items.Count - 1;
        }
    }
}
