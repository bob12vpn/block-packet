# 🛡️ VPN-Hater

🌐 **VPN-Hater** is an innovative, open-source solution aimed at enhancing network security by blocking VPN connections. Distinguished from traditional inline methods, VPN-Hater is both reliable and cost-effective, ensuring uninterrupted network performance even in high-traffic scenarios.

---

## Environment

- Ubuntu 18, 20, 22

---

## 🚀 Key Features

- **💡 Out-of-Path Method**: Guarantees uninterrupted network performance.
- **🔒 Packet Injection**: Expertly blocks VPNs by injecting termination packets.
- **🔄 Supports Multiple Protocols**: Compatible with PPTP, L2TP, TCP Based VPN(OpenVPN TCP), and non-encrypted OpenVPN UDP.
- **🏎️ High Traffic Handling**: Maintains consistent speed under heavy load.
- **💸 Cost-Effective Solution**: Minimizes equipment replacement costs.

---

## 🛠️ Usage Guide

### 🌟 Setting Up

1. **Network Environment Setup**: Configure your network for port mirroring.

   ### 1) Using TAP device to mirror packets

      <img width="50%" src="https://github.com/bob12vpn/vpn-hater/assets/138478029/7a3eccf2-b988-4be5-b644-b9f3c2de428f"/>

   ### 2) Using Internet Router with port mirroring function

      <img width="50%" src="https://github.com/bob12vpn/vpn-hater/assets/138478029/2fd93ebd-773e-4f70-aef5-6ad4e6b1e2f9"/>
      
   ### 3) Using Switch with port mirroring function

      <img width="50%" src="https://github.com/bob12vpn/vpn-hater/assets/138478029/8d5f5a99-ba38-4f06-9331-d095b6619db4"/>


2. **Installation**:
      ```c++
      git clone https://github.com/bob12vpn/vpn-hater.git
      sudo apt install build-essential
      sudo apt install libpcap-dev
      make
      ```

3. **Execution**:
      - command
   
      ```bash
      $ sudo ./vpn-hater <mirror interface> <send interface> [sni list txt]
      ```
      
      - example
   
      ```bash
      $ sudo ./vpn-hater eth0 wlan0 sni.txt
      ```
   



### 📡 How It Works

- **PPTP & L2TP**: Strategy involves injecting termination request packets.
- **OpenVPN UDP (Non-Encrypted)**: Employs explicit-exit-notify packet injection.
- **OpenVPN TCP**: Utilizes parsed signature & injects FIN/RST packets.
- **TCP-Based VPNs**: Enhanced blocking via signature parsing.

---

## 📬 Support

🤝 For assistance, contact us at: [do901328@gmail.com](mailto:do901328@gmail.com)

---

## 📸 Example Usage

![image](https://github.com/bob12vpn/vpn-hater/assets/47083922/c9c68c8e-7714-4560-aadb-8c6dd92c9187)

