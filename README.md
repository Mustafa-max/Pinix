# Pinix-Kernel©

<p align="center">
  <b>Linux mimarisinden esinlenilerek sıfırdan geliştirilen, 64-bit ELF tabanlı eğlence ve öğrenme amaçlı işletim sistemi çekirdeği.</b>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Architecture-x86__64%20ELF-blue?style=flat-square" alt="Architecture">
  <img src="https://img.shields.io/badge/Development%20Time-2%20Weeks-orange?style=flat-square" alt="Dev Time">
  <img src="https://img.shields.io/badge/Status-Hobby%2FFor%20Fun-success?style=flat-square" alt="Status">
</p>

---

## 💡 Proje Hakkında

**Pinix Kernel**, tamamen eğlence, merak ve düşük seviyeli sistem programlamasını keşfetmek amacıyla **2 hafta gibi kısa bir sürede** hayata geçirilmiş bağımsız bir çekirdek projesidir. 

Kod tabanında hiçbir hazır veya telifli Linux kodu kullanılmamıştır; bunun yerine Linux'un zarif mimari yapısından ve felsefesinden ilham alınarak sıfırdan tasarlanmıştır.

## ✨ Öne Çıkan Özellikler

* **Mimarî:** 64-bit ELF formatında derleme altyapısı.
* **Donanım Hedefleri:** Intel, AMD, HP ve Dell gibi popüler sistem mimarileri göz önünde bulundurularak tasarlandı.
* **Hızlı Geliştirme:** Sadece 2 haftalık yoğun bir çalışma ve iterasyon süreciyle şekillendirildi.

## 📌 Uyarı ve Amaç

>Bu proje bilgisayarınızda çalıştırmak isterseniz çalışmayabilir. Sebebi çünkü içinde bootloader kodları olmayan saf kernel kod vardır. Ama distro yaparsanız belki çalışabilir.
> Bu proje tamamen **eğlence ve hobisel** amaçlarla geliştirilmektedir. Üretim ortamında kullanım için tasarlanmamıştır, düşük seviyeli işletim sistemi geliştirme deneyidir.eğer projeyi indirirseniz kernel/main.c kodunu istediğiniz şekilde değiştirin. Değiştirmeyi unutmayın! :D ama eğer çalıştırırsanız ekranda muhtemelen garip şeyler olur veya gözükmez. Bu yüzden C/C++ ve ASM dili bilmeniz gerekir ve bu dilleri kullanarak ayarlama yapmanız gerekir. Kendi PC özelliklerine göre ayarlayın ama distro yaparsanız açık veya kapalı kaynak olsun farketmez. Biraz sorunlar var ama onun dışında iyi yani

