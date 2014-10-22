using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace Crystal {
    public partial class MainForm : Form {
        // ============================= Variables =================================
        public CrystalDevice_t CrystalDevice;
        private Filter_t IFilter;
        private string RXString;

        #region ============================= Init / deinit ============================
        public MainForm()
        {
            InitializeComponent();
            CrystalDevice = new CrystalDevice_t();
            CrystalDevice.ComPort = serialPort;
            IFilter = new Filter_t();
            comboBoxFdiskr.SelectedIndex = 9;
            comboBoxBits.SelectedIndex = 15;
            comboBox_FIR_order.SelectedIndex = 0;
            comboBox_IIR_order.SelectedIndex = 0;
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            timerHeartBeat.Enabled = false;
            try
            {
                CrystalDevice.ComPort.Close();
                CrystalDevice.ComPort = null;
                serialPort.Dispose();
            }
            catch (System.Exception ex)
            {
                CrystalDevice.LastError = ex.Message;
            }
            CrystalDevice.IsConnected = false;
        }
        #endregion 

        #region ======================== Common controls ============================
        // Send Fdiskr and BitWide to stand
        private void comboBoxFdiskr_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (!CrystalDevice.IsConnected) return;
            //CrystalDevice.SendPacket(Cmd.FDiskr, (byte)(comboBoxFdiskr.SelectedIndex + 1));
        }
        private void comboBoxBits_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (!CrystalDevice.IsConnected) return;
            //CrystalDevice.SendPacket(Cmd.BitWide, (byte)(comboBoxBits.SelectedIndex + 1));
        }

        // Check input
        private bool ParseCoefInput(object sender, out decimal Value)
        {
            TextBox Ftb = (TextBox)sender;
            if (!Decimal.TryParse(Ftb.Text, out Value))
            {
                Ftb.BackColor = Color.Red;
                return false;
            }
            else
            {
                Ftb.BackColor = SystemColors.Window;
                return true;
            }
        }
        private void TextBoxCoef_Validating(object sender, CancelEventArgs e)
        {
            decimal FDummy;
            ParseCoefInput(sender, out FDummy);
        }
        private void TextBoxCoef_KeyPress(object sender, KeyEventArgs e)
        {
            decimal FDummy;
            ParseCoefInput(sender, out FDummy);
        }

        // Clear coefs
        private void button_ClearCoefs_Click(object sender, EventArgs e)
        {
            if (tabControl1.SelectedIndex == 0) // FIR
            {
                foreach (Control c in groupBoxFIR_coefs.Controls)
                    if (c is TextBox)
                        if(c.Name != "textBox_FIR_a0") c.Text = "0";
                        else c.Text = "1";
            }
            else if (tabControl1.SelectedIndex == 1)// IIR
            {
                foreach (Control c in groupBoxIIR_coefs.Controls)
                    if (c is TextBox)
                        if (c.Name != "textBox_IIR_a0" && c.Name != "textBox_IIR_b0") c.Text = "0";
                        else c.Text = "1";
            }
        }

        private int GetCoefN(Control c, out string AB)
        {
            AB = "a";
            if (!(c is TextBox)) return -1;
            string IName = c.Name;
            if (!IName.Contains("IR_")) return -2;
            AB = IName.Substring(IName.Length - 2, 1);
            string INumStr = IName.Substring(IName.Length - 1);
            int INum;
            if (int.TryParse(INumStr, out INum)) return INum;
            else return -3;            
        }

        // Send coefs to stand
        private void buttonApply_Click(object sender, EventArgs e)
        {
            if (!CrystalDevice.IsConnected) return;
            string AB;
            GroupBox grb;
            if (tabControl1.SelectedIndex == 0) // FIR
            {
                IFilter.Kind = 0; // FIR
                IFilter.Order = (byte)(comboBox_FIR_order.SelectedIndex);
                grb = groupBoxFIR_coefs;
            }
            else // IIR
            {
                IFilter.Kind = 1; // IIR
                IFilter.Order = (byte)(comboBox_IIR_order.SelectedIndex);
                grb = groupBoxIIR_coefs;
            }

            foreach (Control c in grb.Controls)
            {
                int INum = GetCoefN(c, out AB);
                if (INum >= 0 && INum <= IFilter.Order)
                {
                    // Try to parse input
                    if (AB == "a")
                        if (!ParseCoefInput(c, out IFilter.An[INum]))
                        {
                            return;
                        }// if not parse
                    if (AB == "b")
                        if (!ParseCoefInput(c, out IFilter.Bn[INum]))
                        {
                            return;
                        }// if not parse
                }//if num>0
            }// foreach

            // **** Filter coefs are here, now send'em all ****
            timerHeartBeat.Enabled = false;
            // Clear filter memory
            // CrystalDevice.SendPacket(Cmd.FilterClear, 0);
            // Send filter kind
            // CrystalDevice.SendPacket(Cmd.FilterKind, IFilter.Kind);
            // Send An: An if n<=order, 0 otherwise
/*            for (byte i = 0; i < 10; i++)
            {
                if (i <= IFilter.Order)
                    CrystalDevice.SendPacket(Cmd.FilterAn, i, IFilter.ScaledCoefLo(IFilter.An[i]), IFilter.ScaledCoefHi(IFilter.An[i]));
                else
                    CrystalDevice.SendPacket(Cmd.FilterAn, i, 0, 0);
            }//for
            // if IIR, send Bn too
            if (IFilter.Kind == 1) {
                for (byte i = 0; i < 7; i++)    // Send Bn: Bn if n<=order, 0 otherwise
                {
                    if (i <= IFilter.Order)
                        CrystalDevice.SendPacket(Cmd.FilterBn, i, IFilter.ScaledCoefLo(IFilter.Bn[i]), IFilter.ScaledCoefHi(IFilter.Bn[i]));
                    else
                        CrystalDevice.SendPacket(Cmd.FilterBn, i, 0, 0);
                }//for
            }//if IIR
 */
            timerHeartBeat.Enabled = true;
        }
        #endregion

        #region ======================== FIR & IIR controls ============================
        private void comboBox_FIR_order_SelectedIndexChanged(object sender, EventArgs e)
        {
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

        private void comboBox_IIR_order_SelectedIndexChanged(object sender, EventArgs e)
        {
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
            // Check device if connected
            if(CrystalDevice.IsConnected) {
                /*
                // Ping device to check if it is still ok
                if(CrystalDevice.Ping()) return;   // Nothing to do if it's ok
                else 
                {
                    CrystalDevice.IsConnected = false;
                    // Show "Not connected"
                    labelStendState.Text = "Стенд не подключен";
                    pictureBoxStendState.Image = imageList1.Images[1];
                    return; // Try to connect next time
                } //else
                 */
            } //if is connected
            else { // Not connected, try to connect
                serialPort.Close();
                // First, get ports
                string[] SPorts = System.IO.Ports.SerialPort.GetPortNames();
                int NPorts = SPorts.Length;
                if(NPorts == 0) {
                    labelStendState.Text = "Стенд не подключен";
                    pictureBoxStendState.Image = imageList1.Images[1];
                    labelHelp.Text = "В системе нет ни одного последовательного порта. Если стенд подключен, то, вероятно, не установлены драйверы FT232.";
                }
                else {
                    // Iterate all discovered ports
                    for(int i = 0; i < NPorts; i++) {
                        serialPort.PortName = SPorts[i];
                        try {
                            serialPort.Open();
                            // Try to ping device
                            if(CrystalDevice.Ping() == 0) { // Device found
                                CrystalDevice.IsConnected = true;
                                labelStendState.Text = "Стенд подключен";
                                labelHelp.Text = "Стенд подключен и готов к работе.\r\tВыберите нужный тип фильтра, задайте его порядок и введите коэффициенты. После этого нажмите кнопку 'Применить настройки', чтобы передать данные стенду и включить фильтр.";
                                pictureBoxStendState.Image = imageList1.Images[0];
                                return; // All right, nothing to do here
                            }
                            else serialPort.Close(); // Try next port
                        } //try
                        catch(System.Exception ex) {
                            labelHelp.Text = ex.Message;
                            serialPort.Close();
                        }
                    } //for
                    // Silence answers our cries
                    labelHelp.Text = "Ни на одном из имеющихся в системе последовательных портов стенд не обнаружен.";
                    labelStendState.Text = "Стенд не подключен";
                    pictureBoxStendState.Image = imageList1.Images[1];
                } // if(NPorts != 0) 
            }//else
        } //timerHeartBeat_Tick

        #region ============================== Serial RX handler =========================
        private void serialPort_DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            // Discard data if old packet is not handled
            if (CrystalDevice.PacketIn.IsReceived) {
                serialPort.DiscardInBuffer();
                return;
            }

            // Read RX buffer
            int N = serialPort.BytesToRead;
            serialPort.Read(CrystalDevice.PacketIn.PacketArr, CrystalDevice.PacketIn.RcvLength, N);
            CrystalDevice.PacketIn.RcvLength += N;

            RXString = "";
            if (CrystalDevice.PacketIn.PacketArr[3] == 0xEE)
            {
                for (int i = 0; i < CrystalDevice.PacketIn.RcvLength; i++)
                {
                    RXString += CrystalDevice.PacketIn.PacketArr[i].ToString("X2") + ' ';
                }
                RXString += "\r\n";
            }

            // Check packet consistency
            if (CrystalDevice.PacketIn.PacketArr[0] != 0xCE){
                // Wrong byte0
                CrystalDevice.PacketIn.Init();
                return;
            }
            if ((CrystalDevice.PacketIn.RcvLength >1) && (CrystalDevice.PacketIn.PacketArr[1] != 0xAD)) {
                // Wrong byte1
                CrystalDevice.PacketIn.Init();
                return;
            }
            if (CrystalDevice.PacketIn.RcvLength >2) 
            {
                if (CrystalDevice.PacketIn.PacketArr[2] <2) {
                    // Wrong length
                    CrystalDevice.PacketIn.Init();
                    return;
                }
                CrystalDevice.PacketIn.DataLength = (byte)(CrystalDevice.PacketIn.PacketArr[2] - 1);
            }
            // Command ID
            if (CrystalDevice.PacketIn.RcvLength >3) CrystalDevice.PacketIn.CommandID = CrystalDevice.PacketIn.PacketArr[3];
            // Data          
            if (CrystalDevice.PacketIn.RcvLength > 4)
            {
                // Decide how many bytes of data to read. Mean that there can be more than one packet in buffer.
                int FDataNToRead;
                FDataNToRead = Math.Min((byte)(CrystalDevice.PacketIn.RcvLength - 4), CrystalDevice.PacketIn.DataLength);
                // Read data
                for (int i = 0; i < FDataNToRead; i++)
                    CrystalDevice.PacketIn.Data[i] = CrystalDevice.PacketIn.PacketArr[i+4];
                // Check if packet has been received completely
                if ((byte)(CrystalDevice.PacketIn.RcvLength - 4) >= CrystalDevice.PacketIn.DataLength)
                    CrystalDevice.PacketIn.IsReceived = true;
            }// data
        }
        #endregion

    } //class MainForm

    #region ========================= Classes =========================
    public class Packet_t {
        public byte[] PacketArr;
        // RX related variables
        public byte[] Data;
        public byte DataLength;
        public byte CommandID;
        public int RcvLength;
        public bool IsReceived;

        public void Init() {
            RcvLength = 0;
            IsReceived = false;
        }
        // Constructor
        public Packet_t() {
            PacketArr = new byte[128];
            Data = new byte[54];
            Init();
        }
    }

    public class CrystalDevice_t {
        public bool IsConnected;
        public System.IO.Ports.SerialPort ComPort;
        char[] charSeparators;
        public Packet_t PacketIn, PacketOut;
        public string LastError;

        public CrystalDevice_t() {
            IsConnected = false;
            charSeparators = new char[] {',', ' '};
            PacketIn = new Packet_t();
            PacketOut = new Packet_t();
            LastError = "";
        }

        public int SendCmd(string SCmd, out string[] RxTokens) {
            RxTokens = null;
            if(!ComPort.IsOpen) return 1;
            // Send string
            try {
                ComPort.WriteLine(SCmd);
            }
            catch(System.Exception ex) {
                ComPort.Close();
                IsConnected = false;
                LastError = ex.Message;
            }
            // Wait answer
            ComPort.ReadLine()
            for(int i = 0; i < 10; i++) {
                System.Threading.Thread.Sleep(10);
                if(PacketIn.IsReceived) {
                    // Check if packet is ACK
                    if (PacketIn.CommandID == 0xAC && PacketIn.Data[0] == 0xCC)
                    {
                        // Reset PacketIn
                        PacketIn.Init();
                        return true;
                    }
                    else break;
                }
            }
            return false;
        }

            RxTokens = SCmd.Split(charSeparators, StringSplitOptions.RemoveEmptyEntries); // DEBUG
            return 0;
        }

        public int Ping() {
            string[] RxTokens;
            if(SendCmd("#Ping", out RxTokens) == 0) {
                if(RxTokens.GetLength(0) == 2) {
                    if(RxTokens[0] == "#Ack" && RxTokens[1] == "0") return 0;
                }
            }
            return 1;
        }

        public void SendPacket(byte ACmd, params byte[] AData) {
            if(!ComPort.IsOpen) return;
            // Prepare input packet
            PacketIn.Init(); 
            // Prepare packet
            PacketOut.PacketArr[0] = 0xCE;  //}
            PacketOut.PacketArr[1] = 0xAD;  //} Form start of packet: CEAD
            PacketOut.PacketArr[2] = (byte)(AData.Length + 1); // Length: Cmd ID and data bytes
            PacketOut.PacketArr[3] = ACmd;
            // Add data
            for (int i = 0; i < AData.Length; i++) PacketOut.PacketArr[4 + i] = AData[i];
            // Try to send
            try
            {
                ComPort.Write(PacketOut.PacketArr, 0, 2 + 1 + 1 + AData.Length); // 2 bytes of start word, 1 of length, 1 of cmd ID
            }
            catch (System.Exception ex)
            {
                //if(ComPort.IsOpen) 
                    ComPort.Close();
                IsConnected = false;
                LastError = ex.Message;
            }
        }

        public bool AckReceived()
        {
            for (int i = 0; i < 10; i++)
            {
                System.Threading.Thread.Sleep(10);
                if (PacketIn.IsReceived)
                {
                    // Check if packet is ACK
                    if (PacketIn.CommandID == 0xAC && PacketIn.Data[0] == 0xCC)
                    {
                        // Reset PacketIn
                        PacketIn.Init();
                        return true;
                    }
                    else break;
                }
            }
            return false;
        }
    }// CrystalDevice_t

    public class Filter_t
    {
        public byte Kind, Order;
        public decimal[] An, Bn;
         
        public Filter_t()
        {
            Kind = 0;
            Order = 0;
            An = new decimal[10];
            Bn = new decimal[7];
        }
        public byte ScaledCoefHi(decimal ACoef)
        {
            Int32 FInt = (Int32)(ACoef * 4096);
            UInt16 FUint16 = (UInt16)(FInt & 0x0000FFFF);
            return (byte)((FUint16 >> 8) & 0x00FF);
        }
        public byte ScaledCoefLo(decimal ACoef)
        {
            Int32 FInt = (Int32)(ACoef * 4096);
            UInt16 FUint16 = (UInt16)(FInt & 0x0000FFFF);
            return (byte)( FUint16       & 0x00FF);
        }
    } 
    #endregion
}
