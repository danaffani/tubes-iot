# SAS (Smart Alarm System)

## IoT Camera Monitoring with ESP32-CAM, Supabase, and Vercel

Proyek ini merupakan sistem monitoring menggunakan **ESP32-CAM** yang mengirimkan data atau gambar ke Supabase, lalu ditampilkan melalui halaman web yang di-host di Vercel.

Seluruh file berada pada root repository dan dapat dibuka melalui tautan berikut:

- [`Tubes_IoT.ino`](./Tubes_IoT.ino)
- [`index.html`](./index.html)
- [`vercel.json`](./vercel.json)

---

## 📁 Penjelasan File

### [`Tubes_IoT.ino`](./Tubes_IoT.ino)
Berisi program yang dijalankan di **ESP32-CAM**, Program ini menangani seluruh proses pada alat yang terhubung biar bekerja hingga komunikasi data ke backend.

---

### [`index.html`](./index.html)
Merupakan **halaman website** yang menampilkan hasil dari ESP32-CAM. File ini menjadi antarmuka utama yang diakses pengguna.

---

### [`vercel.json`](./vercel.json)
File konfigurasi untuk **Vercel**

---
