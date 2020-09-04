﻿namespace csharp_test_client
{
    partial class mainForm
    {
        /// <summary>
        /// 필수 디자이너 변수입니다.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 사용 중인 모든 리소스를 정리합니다.
        /// </summary>
        /// <param name="disposing">관리되는 리소스를 삭제해야 하면 true이고, 그렇지 않으면 false입니다.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form 디자이너에서 생성한 코드

        /// <summary>
        /// 디자이너 지원에 필요한 메서드입니다. 
        /// 이 메서드의 내용을 코드 편집기로 수정하지 마세요.
        /// </summary>
        private void InitializeComponent()
        {
            this.btnDisconnect = new System.Windows.Forms.Button();
            this.btnConnect = new System.Windows.Forms.Button();
            this.groupBox5 = new System.Windows.Forms.GroupBox();
            this.textBoxPort = new System.Windows.Forms.TextBox();
            this.label10 = new System.Windows.Forms.Label();
            this.checkBoxLocalHostIP = new System.Windows.Forms.CheckBox();
            this.textBoxIP = new System.Windows.Forms.TextBox();
            this.label9 = new System.Windows.Forms.Label();
            this.labelStatus = new System.Windows.Forms.Label();
            this.listBoxLog = new System.Windows.Forms.ListBox();
            this.chatListBox = new System.Windows.Forms.ListBox();
            this.loginBox = new System.Windows.Forms.GroupBox();
            this.idLabel = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.idTextBox = new System.Windows.Forms.TextBox();
            this.pwTextBox = new System.Windows.Forms.TextBox();
            this.loginButton = new System.Windows.Forms.Button();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.roomNumTextBox = new System.Windows.Forms.TextBox();
            this.roomNumButton = new System.Windows.Forms.Button();
            this.userListBox = new System.Windows.Forms.ListBox();
            this.groupBox5.SuspendLayout();
            this.loginBox.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // btnDisconnect
            // 
            this.btnDisconnect.Font = new System.Drawing.Font("맑은 고딕", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.btnDisconnect.Location = new System.Drawing.Point(421, 44);
            this.btnDisconnect.Name = "btnDisconnect";
            this.btnDisconnect.Size = new System.Drawing.Size(88, 26);
            this.btnDisconnect.TabIndex = 29;
            this.btnDisconnect.Text = "접속 끊기";
            this.btnDisconnect.UseVisualStyleBackColor = true;
            this.btnDisconnect.Click += new System.EventHandler(this.btnDisconnect_Click);
            // 
            // btnConnect
            // 
            this.btnConnect.Font = new System.Drawing.Font("맑은 고딕", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.btnConnect.Location = new System.Drawing.Point(420, 16);
            this.btnConnect.Name = "btnConnect";
            this.btnConnect.Size = new System.Drawing.Size(88, 26);
            this.btnConnect.TabIndex = 28;
            this.btnConnect.Text = "접속하기";
            this.btnConnect.UseVisualStyleBackColor = true;
            this.btnConnect.Click += new System.EventHandler(this.btnConnect_Click);
            // 
            // groupBox5
            // 
            this.groupBox5.Controls.Add(this.textBoxPort);
            this.groupBox5.Controls.Add(this.label10);
            this.groupBox5.Controls.Add(this.checkBoxLocalHostIP);
            this.groupBox5.Controls.Add(this.textBoxIP);
            this.groupBox5.Controls.Add(this.label9);
            this.groupBox5.Location = new System.Drawing.Point(12, 12);
            this.groupBox5.Name = "groupBox5";
            this.groupBox5.Size = new System.Drawing.Size(403, 52);
            this.groupBox5.TabIndex = 27;
            this.groupBox5.TabStop = false;
            this.groupBox5.Text = "Socket 더미 클라이언트 설정";
            // 
            // textBoxPort
            // 
            this.textBoxPort.Location = new System.Drawing.Point(225, 20);
            this.textBoxPort.MaxLength = 6;
            this.textBoxPort.Name = "textBoxPort";
            this.textBoxPort.Size = new System.Drawing.Size(51, 21);
            this.textBoxPort.TabIndex = 18;
            this.textBoxPort.Text = "32452";
            this.textBoxPort.WordWrap = false;
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(163, 24);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(61, 12);
            this.label10.TabIndex = 17;
            this.label10.Text = "포트 번호:";
            // 
            // checkBoxLocalHostIP
            // 
            this.checkBoxLocalHostIP.AutoSize = true;
            this.checkBoxLocalHostIP.Checked = true;
            this.checkBoxLocalHostIP.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxLocalHostIP.Location = new System.Drawing.Point(285, 24);
            this.checkBoxLocalHostIP.Name = "checkBoxLocalHostIP";
            this.checkBoxLocalHostIP.Size = new System.Drawing.Size(103, 16);
            this.checkBoxLocalHostIP.TabIndex = 15;
            this.checkBoxLocalHostIP.Text = "localhost 사용";
            this.checkBoxLocalHostIP.UseVisualStyleBackColor = true;
            // 
            // textBoxIP
            // 
            this.textBoxIP.Location = new System.Drawing.Point(68, 19);
            this.textBoxIP.MaxLength = 6;
            this.textBoxIP.Name = "textBoxIP";
            this.textBoxIP.Size = new System.Drawing.Size(87, 21);
            this.textBoxIP.TabIndex = 11;
            this.textBoxIP.Text = "0.0.0.0";
            this.textBoxIP.WordWrap = false;
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(6, 23);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(61, 12);
            this.label9.TabIndex = 10;
            this.label9.Text = "서버 주소:";
            // 
            // labelStatus
            // 
            this.labelStatus.AutoSize = true;
            this.labelStatus.Location = new System.Drawing.Point(10, 403);
            this.labelStatus.Name = "labelStatus";
            this.labelStatus.Size = new System.Drawing.Size(111, 12);
            this.labelStatus.TabIndex = 40;
            this.labelStatus.Text = "서버 접속 상태: ???";
            // 
            // listBoxLog
            // 
            this.listBoxLog.FormattingEnabled = true;
            this.listBoxLog.HorizontalScrollbar = true;
            this.listBoxLog.ItemHeight = 12;
            this.listBoxLog.Location = new System.Drawing.Point(12, 264);
            this.listBoxLog.Name = "listBoxLog";
            this.listBoxLog.Size = new System.Drawing.Size(397, 136);
            this.listBoxLog.TabIndex = 41;
            // 
            // chatListBox
            // 
            this.chatListBox.FormattingEnabled = true;
            this.chatListBox.ItemHeight = 12;
            this.chatListBox.Location = new System.Drawing.Point(12, 114);
            this.chatListBox.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.chatListBox.Name = "chatListBox";
            this.chatListBox.Size = new System.Drawing.Size(397, 136);
            this.chatListBox.TabIndex = 52;
            // 
            // loginBox
            // 
            this.loginBox.Controls.Add(this.loginButton);
            this.loginBox.Controls.Add(this.pwTextBox);
            this.loginBox.Controls.Add(this.idTextBox);
            this.loginBox.Controls.Add(this.label1);
            this.loginBox.Controls.Add(this.idLabel);
            this.loginBox.Location = new System.Drawing.Point(12, 59);
            this.loginBox.Name = "loginBox";
            this.loginBox.Size = new System.Drawing.Size(397, 49);
            this.loginBox.TabIndex = 53;
            this.loginBox.TabStop = false;
            this.loginBox.Text = "로그인";
            // 
            // idLabel
            // 
            this.idLabel.AutoSize = true;
            this.idLabel.Location = new System.Drawing.Point(6, 25);
            this.idLabel.Name = "idLabel";
            this.idLabel.Size = new System.Drawing.Size(41, 12);
            this.idLabel.TabIndex = 0;
            this.idLabel.Text = "아이디";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(150, 25);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(53, 12);
            this.label1.TabIndex = 1;
            this.label1.Text = "비밀번호";
            // 
            // idTextBox
            // 
            this.idTextBox.Location = new System.Drawing.Point(53, 21);
            this.idTextBox.Name = "idTextBox";
            this.idTextBox.Size = new System.Drawing.Size(91, 21);
            this.idTextBox.TabIndex = 2;
            this.idTextBox.Text = "cjwoov";
            // 
            // pwTextBox
            // 
            this.pwTextBox.Location = new System.Drawing.Point(209, 21);
            this.pwTextBox.Name = "pwTextBox";
            this.pwTextBox.Size = new System.Drawing.Size(91, 21);
            this.pwTextBox.TabIndex = 3;
            this.pwTextBox.Text = "wlsdn";
            this.pwTextBox.UseSystemPasswordChar = true;
            // 
            // loginButton
            // 
            this.loginButton.Location = new System.Drawing.Point(313, 19);
            this.loginButton.Name = "loginButton";
            this.loginButton.Size = new System.Drawing.Size(75, 23);
            this.loginButton.TabIndex = 4;
            this.loginButton.Text = "로그인";
            this.loginButton.UseVisualStyleBackColor = true;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.userListBox);
            this.groupBox1.Controls.Add(this.roomNumButton);
            this.groupBox1.Controls.Add(this.roomNumTextBox);
            this.groupBox1.Location = new System.Drawing.Point(420, 76);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(92, 324);
            this.groupBox1.TabIndex = 54;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "방";
            // 
            // roomNumTextBox
            // 
            this.roomNumTextBox.Location = new System.Drawing.Point(7, 21);
            this.roomNumTextBox.Name = "roomNumTextBox";
            this.roomNumTextBox.Size = new System.Drawing.Size(79, 21);
            this.roomNumTextBox.TabIndex = 0;
            this.roomNumTextBox.Text = "1";
            // 
            // roomNumButton
            // 
            this.roomNumButton.Location = new System.Drawing.Point(7, 49);
            this.roomNumButton.Name = "roomNumButton";
            this.roomNumButton.Size = new System.Drawing.Size(79, 23);
            this.roomNumButton.TabIndex = 1;
            this.roomNumButton.Text = "입장";
            this.roomNumButton.UseVisualStyleBackColor = true;
            // 
            // userListBox
            // 
            this.userListBox.FormattingEnabled = true;
            this.userListBox.ItemHeight = 12;
            this.userListBox.Location = new System.Drawing.Point(7, 79);
            this.userListBox.Name = "userListBox";
            this.userListBox.Size = new System.Drawing.Size(79, 232);
            this.userListBox.TabIndex = 2;
            // 
            // mainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(524, 422);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.loginBox);
            this.Controls.Add(this.chatListBox);
            this.Controls.Add(this.labelStatus);
            this.Controls.Add(this.listBoxLog);
            this.Controls.Add(this.btnDisconnect);
            this.Controls.Add(this.btnConnect);
            this.Controls.Add(this.groupBox5);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
            this.Name = "mainForm";
            this.Text = "네트워크 테스트 클라이언트";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.mainForm_FormClosing);
            this.Load += new System.EventHandler(this.mainForm_Load);
            this.groupBox5.ResumeLayout(false);
            this.groupBox5.PerformLayout();
            this.loginBox.ResumeLayout(false);
            this.loginBox.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button btnDisconnect;
        private System.Windows.Forms.Button btnConnect;
        private System.Windows.Forms.GroupBox groupBox5;
        private System.Windows.Forms.TextBox textBoxPort;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.CheckBox checkBoxLocalHostIP;
        private System.Windows.Forms.TextBox textBoxIP;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label labelStatus;
        private System.Windows.Forms.ListBox listBoxLog;
        private System.Windows.Forms.ListBox chatListBox;
        private System.Windows.Forms.GroupBox loginBox;
        private System.Windows.Forms.Button loginButton;
        private System.Windows.Forms.TextBox pwTextBox;
        private System.Windows.Forms.TextBox idTextBox;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label idLabel;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.ListBox userListBox;
        private System.Windows.Forms.Button roomNumButton;
        private System.Windows.Forms.TextBox roomNumTextBox;
    }
}

