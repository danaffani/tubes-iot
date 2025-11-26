# SAS (Smart Alarm System)

**IoT Sistem Alarm low-budget menggunakan ESP32-CAM, Supabase, dan Vercel**

Proyek ini merupakan sistem monitoring menggunakan **ESP32-CAM** yang mengirimkan data atau gambar ke database **Supabase**, lalu ditampilkan melalui halaman web yang di-host di **Vercel**.

Seluruh file berada pada root repository dan dapat dibuka melalui tautan berikut:

[`Tubes_IoT.ino`](./Tubes_IoT.ino)
Berisi program yang dijalankan di **ESP32-CAM**, Program ini menangani seluruh proses pada alat yang terhubung biar bekerja hingga komunikasi data ke backend.

[`index.html`](./index.html)
Merupakan **halaman website** yang menampilkan hasil dari ESP32-CAM. File ini menjadi antarmuka utama yang diakses pengguna.

[`vercel.json`](./vercel.json)
File konfigurasi untuk **Vercel**

---

**Dibuat oleh:**
Mahasiswa Informatika UNS, tugas mata kuliah Teknologi IoT, Kelompok 4:
- Dana Affan Rabbani (L0122041)
- Irfan Adi Prasetya (L0122079)
- M. Arva Zaky W. P. (L0122104)

**Dosen Pengampu:**
- Abdul Aziz S.Kom., M.Cs. (NIP. 198104132005011001)
