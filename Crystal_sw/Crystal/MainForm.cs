using System;
using System.Drawing;
using System.Windows.Forms;

namespace Crystal
{
    public enum Rslt {OK=0, Failure=1, CmdError=2};

    public partial class MainForm : Form {
        // ============================= Variables =================================
        public CrystalDevice_t CrystalDevice;
        //private string RXString;

        #region ============================= Init / deinit ============================
        public MainForm() {
            InitializeComponent();
            CrystalDevice = new CrystalDevice_t();
            CrystalDevice.ComPort = serialPort;
            cbFreqSampling.SelectedIndex = 3;
            cbResolution.SelectedIndex = cbResolution.Items.Count - 1;
            InformNoInfoOnSettings();
            comboBox_FIR_order.SelectedIndex = 0;
            comboBox_IIR_order.SelectedIndex = 0;
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e) {
            timerHeartBeat.Enabled = false;
            try {
                CrystalDevice.ComPort.Close();
                CrystalDevice.ComPort = null;
                serialPort.Dispose();
            }
            catch (System.Exception ex) {
                CrystalDevice.LastError = ex.Message;
            }
            CrystalDevice.IsConnected = false;
        }
        #endregion 

        // ==== Interface ====
        void DisplayFailure(string ErrorText) {
            StatusLabel.Text = ErrorText;
        }
        private void InformNewSettings() {
            StatusLabelNewSettings.Text = "Новые настройки не применены";
            StatusLabelNewSettings.Image = imageList1.Images[2];
        }
        private void InformSettingsSent() {
            StatusLabelNewSettings.Text = "Настройки переданы в стенд";
            StatusLabelNewSettings.Image = imageList1.Images[3];
        }
        private void InformSendingFailure() {
            StatusLabelNewSettings.Text = "Сбой передачи";
            StatusLabelNewSettings.Image = imageList1.Images[2];
        }
        private void InformNoInfoOnSettings() {
            StatusLabelNewSettings.Text = "";
            StatusLabelNewSettings.Image = null;
        }

        #region ======================== Common controls ============================
        // Send FSampling and BitResolution to stand
        private void comboBoxFdiskr_SelectedIndexChanged(object sender, EventArgs e) {
            if(!CrystalDevice.IsConnected) return;
            int Freq = Convert.ToInt16(cbFreqSampling.Text);
            Freq *= 1000;
            if(CrystalDevice.Command("SetSmplFreq " + Freq.ToString()) != Rslt.OK) DisplayFailure("Сбой при обмене данными");
        }
        private void comboBoxBits_SelectedIndexChanged(object sender, EventArgs e) {
            if(!CrystalDevice.IsConnected) return;
            int Resolution = Convert.ToInt16(cbResolution.Text);
            if(CrystalDevice.Command("SetResolution " + Resolution.ToString()) != Rslt.OK) DisplayFailure("Сбой при обмене данными");
        }

        private void tabControl1_SelectedIndexChanged(object sender, EventArgs e) {
            InformNoInfoOnSettings();
        }
        
        // Check input of FIR/IIR
        private Rslt CheckFirIirCoef(TextBox tb) {
            if (!decimal.TryParse(tb.Text, out decimal Dummy))
            {
                tb.BackColor = Color.Red;
                return Rslt.Failure;
            }
            else
            {
                tb.BackColor = SystemColors.Window;
                return Rslt.OK;
            }
        }

        private void textBox_FIR_a0_TextChanged(object sender, EventArgs e) {
            CheckFirIirCoef((TextBox)sender);
            InformNewSettings();
        }

        // Check input Notch
        private Rslt CheckNotchK(TextBox tb) {
            if (!decimal.TryParse(tb.Text, out decimal k))
            {
                tb.BackColor = Color.Red;
                return Rslt.Failure;
            }
            else
            {
                if (k >= 1)
                {
                    tb.BackColor = Color.Red;
                    return Rslt.Failure;
                }
                else
                {
                    tb.BackColor = SystemColors.Window;
                    return Rslt.OK;
                }
            }
        }
        private void textBox_k1_TextChanged(object sender, EventArgs e) {
            CheckNotchK((TextBox)sender);
            InformNewSettings();
        }

        // Clear coefs
        private void button_ClearCoefs_Click(object sender, EventArgs e) {
            switch(tabControl1.SelectedIndex) { 
                case 0: // FIR
                    foreach(Control c in groupBoxFIR_coefs.Controls) {
                        if(c is TextBox) {
                            if(c.Name != "textBox_FIR_a0") c.Text = "0";
                            else c.Text = "1";
                            CheckFirIirCoef((TextBox)c);
                        }
                    }
                    break;
            
                case 1: // IIR
                    foreach(Control c in groupBoxIIR_coefs.Controls) {
                        if(c is TextBox) {
                            if(c.Name != "textBox_IIR_a0" && c.Name != "textBox_IIR_b0") c.Text = "0";
                            else c.Text = "1";
                            CheckFirIirCoef((TextBox)c);
                        }
                    }
                    break;

                case 2: // Notch
                    textBox_k1.Text = "0";
                    textBox_k2.Text = "0";
                    CheckNotchK(textBox_k1);
                    CheckNotchK(textBox_k2);
                    break;
            } // switch
            InformNewSettings();
        } // ClearCoeffs

        private int GetCoefN(TextBox tb, out string AB) {
            AB = "a";
            string IName = tb.Name;
            if(!IName.Contains("IR_")) return -2;
            AB = IName.Substring(IName.Length - 2, 1);
            string INumStr = IName.Substring(IName.Length - 1);
            if (int.TryParse(INumStr, out int INum)) return INum;
            else return -3;
        }

        // Send coefs to stand
        private void buttonApply_Click(object sender, EventArgs e) {
            if(!CrystalDevice.IsConnected) return;
            string SCmd = "";
            // Fir and IIR
            if(tabControl1.SelectedIndex < 2) {
                GroupBox grb;
                int Order;
                // FIR 
                if (tabControl1.SelectedIndex == 0) { 
                    Order = comboBox_FIR_order.SelectedIndex;
                    SCmd = "SetupFir ";
                    grb = groupBoxFIR_coefs;
                }
                // IIR 
                else { 
                    Order = comboBox_IIR_order.SelectedIndex;
                    SCmd = "SetupIir ";
                    grb = groupBoxIIR_coefs;
                }
                // Iterate textboxes
                foreach(Control c in grb.Controls) {
                    if(!(c is TextBox)) continue;
                    TextBox tb = (TextBox)c;
                    int INum = GetCoefN(tb, out string AB);
                    if(INum >= 0 && INum <= Order) {
                        if(AB == "a") {
                            if(CheckFirIirCoef(tb) != Rslt.OK) return;
                            else SCmd += "a" + tb.Text.Replace(',', '.') + ",";
                        }
                        else if(AB == "b") {
                            if(INum == 0) continue;
                            if(CheckFirIirCoef(tb) != Rslt.OK) return;
                            else SCmd += "b" + tb.Text.Replace(',', '.') + ",";
                        }
                    }
                } // foreach
                // Remove last colon
                SCmd = SCmd.Remove(SCmd.Length - 1);
            } // if FIR or IIR
            
            // Notch
            else if(tabControl1.SelectedIndex == 2) {
                if(CheckNotchK(textBox_k1) != Rslt.OK || CheckNotchK(textBox_k2) != Rslt.OK) return;
                else SCmd = "SetupNotch " + textBox_k1.Text.Replace(',', '.') + " " + textBox_k2.Text.Replace(',', '.');
            }
            
            // ==== Setup new filter data ====
            timerHeartBeat.Enabled = false;
            if(CrystalDevice.Command(SCmd) == Rslt.OK) InformSettingsSent();
            else InformSendingFailure();
            timerHeartBeat.Enabled = true;
        }
        #endregion

        #region ======================== FIR & IIR controls ============================
        private void comboBox_FIR_order_SelectedIndexChanged(object sender, EventArgs e) {
            if(CrystalDevice.IsConnected) InformNewSettings();
            int FirOrder = comboBox_FIR_order.SelectedIndex;
            label_FIR_a1.Visible = label_FIR_ai1.Visible = textBox_FIR_a1.Visible = FirOrder > 0;
            label_FIR_a2.Visible = label_FIR_ai2.Visible = textBox_FIR_a2.Visible = FirOrder > 1;
            label_FIR_a3.Visible = label_FIR_ai3.Visible = textBox_FIR_a3.Visible = FirOrder > 2;
            label_FIR_a4.Visible = label_FIR_ai4.Visible = textBox_FIR_a4.Visible = FirOrder > 3;
            label_FIR_a5.Visible = label_FIR_ai5.Visible = textBox_FIR_a5.Visible = FirOrder > 4;
            label_FIR_a6.Visible = label_FIR_ai6.Visible = textBox_FIR_a6.Visible = FirOrder > 5;
            label_FIR_a7.Visible = label_FIR_ai7.Visible = textBox_FIR_a7.Visible = FirOrder > 6;
            label_FIR_a8.Visible = label_FIR_ai8.Visible = textBox_FIR_a8.Visible = FirOrder > 7;
            label_FIR_a9.Visible = label_FIR_ai9.Visible = textBox_FIR_a9.Visible = FirOrder > 8;
        }

        private void comboBox_IIR_order_SelectedIndexChanged(object sender, EventArgs e) {
            if(CrystalDevice.IsConnected) InformNewSettings();
            int IirOrder = comboBox_IIR_order.SelectedIndex;
            label_IIR_a1.Visible = label_IIR_ai1.Visible = textBox_IIR_a1.Visible = IirOrder > 0;
            label_IIR_a2.Visible = label_IIR_ai2.Visible = textBox_IIR_a2.Visible = IirOrder > 1;
            label_IIR_a3.Visible = label_IIR_ai3.Visible = textBox_IIR_a3.Visible = IirOrder > 2;
            label_IIR_a4.Visible = label_IIR_ai4.Visible = textBox_IIR_a4.Visible = IirOrder > 3;
            label_IIR_a5.Visible = label_IIR_ai5.Visible = textBox_IIR_a5.Visible = IirOrder > 4;
            label_IIR_a6.Visible = label_IIR_ai6.Visible = textBox_IIR_a6.Visible = IirOrder > 5;

            label_IIR_b1.Visible = label_IIR_bi1.Visible = textBox_IIR_b1.Visible = IirOrder > 0;
            label_IIR_b2.Visible = label_IIR_bi2.Visible = textBox_IIR_b2.Visible = IirOrder > 1;
            label_IIR_b3.Visible = label_IIR_bi3.Visible = textBox_IIR_b3.Visible = IirOrder > 2;
            label_IIR_b4.Visible = label_IIR_bi4.Visible = textBox_IIR_b4.Visible = IirOrder > 3;
            label_IIR_b5.Visible = label_IIR_bi5.Visible = textBox_IIR_b5.Visible = IirOrder > 4;
            label_IIR_b6.Visible = label_IIR_bi6.Visible = textBox_IIR_b6.Visible = IirOrder > 5;
        }
        #endregion

        private void timerHeartBeat_Tick(object sender, EventArgs e) {
            // Check if device connected
            if(CrystalDevice.IsConnected) {
                // Ping device to check if it is still ok
                if(CrystalDevice.Ping() == Rslt.OK) return;   // Nothing to do if it's ok
                else {
                    CrystalDevice.IsConnected = false;
                    // Show "Not connected"
                    StatusLabel.Text = "Стенд не подключен";
                    StatusLabel.Image = imageList1.Images[1];
                    InformNoInfoOnSettings();
                    return; // Try to connect next time
                } //else
            } //if is connected
            else { // Not connected, try to connect
                try {
                    serialPort.Close();
                }
                catch{ }
                // First, get port names
                string[] SPorts = System.IO.Ports.SerialPort.GetPortNames();
                int NPorts = SPorts.Length;
                if(NPorts == 0) {
                    StatusLabel.Text = "Стенд не подключен";
                    StatusLabel.Image = imageList1.Images[1];
                    labelHelp.Text = "В системе нет ни одного последовательного порта. Если стенд подключен, то, вероятно, не установлены драйверы USB-Serial.";
                }
                else {
                    // Iterate all discovered ports
                    for(int i = 0; i < NPorts; i++) {
                        serialPort.PortName = SPorts[i];
                        try {
                            serialPort.Open();
                            // Try to ping device
                            if(CrystalDevice.Ping() == Rslt.OK) { // Device found
                                CrystalDevice.IsConnected = true;
                                StatusLabel.Text = "Стенд подключен";
                                StatusLabel.Image = imageList1.Images[0];
                                labelHelp.Text = "Стенд подключен и готов к работе."+ Environment.NewLine +"Выберите нужный тип фильтра, задайте его порядок, введите коэффициенты. После этого нажмите кнопку 'Применить настройки' или 'Enter', чтобы передать данные стенду и включить фильтр.";
                                return; // All right, nothing to do here
                            }
                            else serialPort.Close(); // Try next port
                        } // try
                        catch(System.Exception ex) {
                            labelHelp.Text = ex.Message;
                            serialPort.Close();
                        }
                    } //for
                    // Silence answers our cries
                    labelHelp.Text = "Ни на одном из имеющихся в системе последовательных портов стенд не обнаружен.";
                    StatusLabel.Text = "Стенд не подключен";
                    StatusLabel.Image = imageList1.Images[1];
                } // if(NPorts != 0) 
            }//else
        } //timerHeartBeat_Tick

    } //class MainForm

    #region ========================= Classes =========================
    public class CrystalDevice_t {
        // ==== Private ====
        Rslt SendAndGetReply(string SCmd, out string SReply) {
            SReply = "";
            if(!ComPort.IsOpen) return Rslt.Failure;
            // Send string
            try {
                ComPort.WriteLine(SCmd);
                SReply = ComPort.ReadLine().Trim();
                return Rslt.OK;
            }
            catch(System.Exception ex) {
                IsConnected = false;
                LastError = ex.Message;
                try {
                    ComPort.Close();
                }
                catch(System.Exception ex2) {
                    LastError = ex2.Message;
                }
            }
            return Rslt.Failure;
        }
        
        // ==== Public ====
        public bool IsConnected;
        public System.IO.Ports.SerialPort ComPort;
        public string LastError;

        public CrystalDevice_t() {
            IsConnected = false;
            LastError = "";
        }

        // ==== Commands ===
        public Rslt Ping() {
            if (SendAndGetReply("Ping", out string SReply) == Rslt.OK) {
                if (SReply.Equals("Ack 0", StringComparison.OrdinalIgnoreCase)) return Rslt.OK;
            }
            return Rslt.Failure;
        }

        public Rslt Command(string SCmd) {
            if (SendAndGetReply(SCmd, out string SReply) == Rslt.OK)
            {
                if (SReply.Equals("Ack 0", StringComparison.OrdinalIgnoreCase)) return Rslt.OK;
                else return Rslt.CmdError;
            }
            return Rslt.Failure;
        }
    } // CrystalDevice_t
    #endregion
}
