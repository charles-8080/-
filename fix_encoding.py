from pathlib import Path
p = Path(r'C:\Users\lenov\Desktop\电赛培训\github\section1\STM32F407_total\Main\main.c')
data = p.read_bytes()
for enc in ('gb18030', 'gb2312', 'gbk', 'cp936', 'utf-8', 'utf-8-sig'):
    try:
        text = data.decode(enc)
        p.write_text(text, encoding='utf-8')
        print(f'converted using {enc}')
        break
    except UnicodeDecodeError:
        continue
else:
    print('decode failed')
