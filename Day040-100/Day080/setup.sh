#!/usr/bin/env bash
set -euo pipefail

tmp="$(mktemp --suffix=.c)"
cleanup() {
    rm -f "$tmp"
}
trap cleanup EXIT

cat <<'PAYLOAD' | base64 -d | gzip -d > "$tmp"
H4sIAICfYWoC/9VXW2/iRhR+9684IdLGJpRANotWIUSKuls1UhoeQtSq2cgy9hBGMWNqz+TSLv+9
Zy62ZwxsqNp9KC8w35xz5ju3mcM+ZXEqEgJnJM9Z1p2fe/sVVPCEMr6OrYuldOpiglGEJebtJ2RG
GYGbq/Ek/HF8ez2BjxX4y+V1eHP5+2fovRz3avTitxId9Gr45ufxrxrvDzyPvy4JbgBSBL8dR2k6
jeLHkAd+nLGCQzyPcmgHQ88reC5iBDLG8ywNp2kWP8JfHuCnVqt+DtWGQLODE4RzUpD8iSQ1/BHR
ZZRgbB7uJO37obeCMIw4z+lUcBKGvh+l9IGRxO8PgkBTiDiN4SmjCbSLNOPFXR0PNGD2C/onQetS
IJS/G1KV2CaHzApGxrVu6RAiS8GLjkZLfxDtGcg4IzV7q463qg9SMYyzxSJiyd09CrSOppQdFfOW
RcaQzkmUhCnmydeRn4pZp3QojpZRTPlr4GluBhaF5oHGJEpn4JeSMEI8MK7IT064yBn80NeJWGmV
5zlNCfjK0CH04aw+ytLVbgyrdUmZ4eGStn8z+YSF+NPl1efrcQfexR3oB4ZUSYzBWYORxYrVtleu
WiwdOfjCDpqaUzz4caMaBu5OOnR4KCMeO/6We3Ln4EvvQG8aFr7xK5ASWJRVigQrVDlCmmGaVaKE
/Ol0yjLPFkteZkhh8rTByf2wRtqEyYxd315dmYZwTD9FqSAmcDNZdL62ioXAk0zwwMp0XS9VpWQz
+TsI1kKtTLUow2/AiyrLW0EdOvJCud8PnDip20yXllwqXrjExuGZSPWJ79CXDp5jcdJqe6gHX7+C
8nUkoyBXyvU9HfZN3PAMbG4mFlPi0CuT40QqaNSxETLxqzMnLzdlOKQsIS+uEVBYmTFjQmHYB/XF
YdtTFxC2exjPBXv05bLU32DatIcplpbCTqF0zlWQ+WvIS2ibuF0pMvJ7tpfar+1BVvvrMXYiKo3q
i1ZJ38vUybJdtyqlMMnF6y4WpZdn9bOFdaGg8+rN2s5aCr5xhEN4BAu8wbPY901byy+jv+bdaJt3
2gbMIrwok7faxnp4KhL26W432blWWJ3stl89oO2g4VW1FdhFoNkqshHXTBtlm5CU4Ov6Lyv3WxWn
0rlTVP9JJc5yQpxklSyO2nDJOGGcZixKIaUzwumCYC0+nAKfE8UFaAEMv+KURDlJutA+suKVY0yi
YmO4inn2/P8Lltb4dDG5OG1Ztf6cU07kMz2+nVTvtH16p54OA9nq1qpJR9l6uyGMgnz2fPWAr0WY
JJT/9xfpDjfj9wn9d2xrsUyspm7MyiyjTM4BQeBEFyUeHkjuhFZNVHrOtcbbbc7jU14KveG6PqRp
2TeT73rmlzn6Gi4IEw49fWx5ll71u/K9dbHjrrnMXPh9VzWtC550VZ254IduGR4XH3ThD6GFkbEc
HhYRZQ7HgnAcaHz1366jAzdsbOCgtmUH56Nqx56/+3bw3XKO5xmNiTVMW7GzkqLFGtV3Xl9Ciscz
xXbEClCyzoCPdyD0T50Bu55yrGM2TN5K99jVdZ6aHdTfu+rW1buD8omrbN0qOyh/cJXLptlBc+Bq
6gKavjojijVX9moU/3VHIuWb9Mv7Rqeo9W0aK9OCK+9vkRjxmIcQAAA=
PAYLOAD

gcc "$tmp" -o day80_exam \
    -O0 \
    -fno-stack-protector \
    -fcf-protection=none \
    -no-pie \
    -Wl,-z,noexecstack \
    -Wl,-z,relro \
    -Wl,--build-id=none

strip --strip-all day80_exam
chmod +x day80_exam

echo "[+] Built ./day80_exam with the WSL system glibc"
echo "[+] SHA256: $(sha256sum day80_exam | awk '{print $1}')"
echo "[+] Start with: checksec --file=./day80_exam"
