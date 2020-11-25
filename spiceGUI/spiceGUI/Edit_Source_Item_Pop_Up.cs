using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace spiceGUI
{
    public partial class Edit_Source_Item_Pop_Up : Form
    {
        ListBox listbox;
        List<string> component;
        List<string> node;
        public Edit_Source_Item_Pop_Up(ListBox _listbox, List<string> _component, List<string> _node)
        {
            InitializeComponent();
            listbox = _listbox;
            component = _component;
            node = _node;
            int index = listbox.SelectedIndex;
            textBox1.Text = listbox.Items[index].ToString();
        }

        private void EditSourceItem_Load(object sender, EventArgs e)
        {

        }

        private void button1_Click(object sender, EventArgs e)
        {
            int index = listbox.SelectedIndex;
            listbox.Items.RemoveAt(index);
            listbox.Items.Insert(index, textBox1.Text);
        }
    }
}
