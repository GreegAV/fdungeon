#MOBPROGS
#1
if ispc $n
mob oload 3051
give canoe $n
smile $n
endif
~
#3
if ispc $n
emote ��������.
if isgood $n
emote ���������� "�� �� ���� ������������!"
emote ����������� "�� �� �������� ����� ������!"
emote ����������  "�����, �� �����?"
smile $n
else
emote ���������� "��, � �� ����!"
giggle $n
emote ���������� "��, ��� ������ �� �������!"
emote ���������� "���� ��� ������� �� ������? ����� ����������!"
giggle $n
emote ������ ������� �� $n
endif
endif
~
#4
if ispc $n
say ������������� �������� ...
emote ���������� '�� ��� ��������� ? ��� ������ �����'
emote � ������� ����� ������� ��������� ����� ���������
look $n
endif
~
#5
if ispc $n 
    emote ���� '{m������� $n ! ���� ������, ���� �� ������ ����, � ��� ������ {R����!{x'{/ 
    mob transfer $n 12536
endif
~
#6
if ispc $n 
    emote ���� '{m������� $n ! {R������ �� �� �����?!! {W��� ������ ��� �������!{x'{/
    mob transfer $n 12438
    mob force $n toilet
    mob force $n toilet
    mob force $n sigh
    mob force $n rest
endif
~
#7
say '������ ��� ������� ������������...���������!'
~
#8
say {B��� �� ������ ���������...{x
emote ���� ����� ��� {R���{x $n .
if ispc $n
  mob transfer $n 3336
endif
emote ���� ��������� �� ������.
emote ������ ��������.
say ������ ��������� !
giggle
~
#9
if ispc $n
  say ����� �� ���������
  mob cast 'giant strength' $n
  say �! ��� ������� �����!
endif
~
#10
say � ���� �� ������� ? ������� ?
emote ���� ���� � ������� !
if ispc $n
 if hpcnt $n <= 30
   mob force $n north
   mob force $n sleep bed
 endif
endif
~
#11
if ispc $n
 giggle $n
 poke $n
 giggle $n
endif
~
#12
if ispc $n
 say ������� ����, �������� $n !
 say ��� �p��, $n , ������� ��p��� ��p���.
 say ��� ��p��� ���������� ����������!
 say ��� $n ���-�� � ������ �������� �������. !
 say ���� p���� ��������� � area �������.
 say � ���� ���� 13 ����� �� ���������� �������.
say �� �p������ � ����� ����!
endif
~
#13
if ispc $n
   say ������� ����, �������� $n !
 if level $n <= 10
   say ������� �p��������, {R������ ��p������{x , ������ �� �������!
   say � ��� ��p ������ {R������ ��p������{x ,  ���� 5 �������!
   say H�������� �� ��� �p���������� - ���p��, � �� �p������� ��� � ����������!
   say ��� {R������ ���������{x ���-�� � p����� {Y���p����� �����{x!
   say ���� p���� ��������� � area {Y������� �����{x.
  else if level  $n <= 20  and $n > 10
   say ������� �p��������, {RA slaver buyer{x , ������ �� �������!
   say � ��� ��p ������ {RA slaver buyer{x ,  ���� 7 �������!
   say H�������� �� ��� �p���������� - ���p��, � �� �p������� ��� � ����������!
   say ��� the {RA slaver buyer{x ���-�� � p����� {YEntrance to the Slaver's Market{x!
   say ���� p���� ��������� � area {YUnderDark{x.
  else if level $n <= 30 and $n > 20 
   say ������� �p��������, {R���������� ����{x , ������ �� �������!
   say � ��� ��p ������ {R���������� ����{x ,  ���� 9 �������!
   say H�������� �� ��� �p���������� - ���p��, � �� �p������� ��� � ����������!
   say ��� the {R���������� ����{x ���-�� � p����� {Y��� ������������ ���{x!
   say ���� p���� ��������� � area {Y��{x.
  else  if level $n <= 55 and $n > 30 
   say ������� �p��������, {R�������� ����� ������{x, ������ �� �������!
   say � ��� ��p ������ {R�������� ����� ������{x ,  ���� 15 �������!
   say H�������� �� ��� �p���������� - ���p��, � �� �p������� ��� � ����������!
   say ��� the {R�������� ����� ������{x ���-�� � p����� {Y�������� �������{x!
   say ���� p���� ��������� � area {Y���������� ����{x.
 else  if level  $n <= 70 ) and $n > 55 
   say ������� �p��������, {RA drakyri healer{x, ������ �� �������!
   say � ��� ��p ������ {RA drakyri healer{x ,  ���� 25 �������!
   say H�������� �� ��� �p���������� - ���p��, � �� �p������� ��� � ����������!
   say ��� the {RA drakyri healer{x ���-�� � p����� {YWinding path{x!
   say ���� p���� ��������� � area {YDrakyri Isle{x.
 else if level  $n > 70 and $n <= 90
  say ������� �p��������, {R�����{x, ������ �� �������!
  say � ��� ��p ������ {R�����{x ,  ���� 45 �������!
  say H�������� �� ��� �p���������� - ���p��, � �� �p������� ��� � ����������!
  say ��� the {R�����{x ���-�� � p����� {Y���� ����{x!
  say ���� p���� ��������� � area {Y����������{x.
 else  if level $n > 90
  say ������� �p��������, {RAn evil shadow person{x �� �������!
  say � ��� ��p ������ {RAn evil shadow person{x ,  ���� 65 �������!
  say H�������� �� ��� �p���������� - ���p��, � �� �p������� ��� � ����������!
  say ��� the {RAn evil shadow person{x ���-�� � p����� {Y������ ����� �������{x!
  say ���� p���� ��������� � area {YWestLand{x.
endif
endif
endif
endif
endif
endif
endif
 say � ���� ���� 13 ����� �� ���������� �������.
 say �� �p������ � ����� ����!
endif
~
#14
if ispc $n
if sex $n < 2
say ����������� ������ $n!
say ���� ����� �������������� ��������� $n.
else
say ����������� ������ $n!
say ���� ����� �������������� ��������� $n.
endif
say ��� ����� ��������� ���������� �������� travel list.
endif
~
#20
mob transfer $n 30510
~
#21
if ispc $n
mob force $n sleep
mob back $n
endif
~
#22
if ispc $n
sc
say ��� ����� $n ? 
emote ��������� {D������� {R���{x ����� ��� �������� ���� �����.
say {Y������� ����� ���{x
comf $n
endif
"
~
#28
~
#29
~
#50
if ispc $n
if level $n <= 17
say ���������� ������� ������� � � ��� � �������.
endif
endif
~
#51
if ispc $n
say ������, ����� �����.
endif
~
#52
if ispc $n
if level $n <= 17
smile
tell $n ������ � ���������� �������.
scan
tell $n �������, ��� � ������-�� ������� ����, ��� ����� ������.
nod $n
endif
endif
~
#53
if ispc $n
if level $n <= 17
cry $n
tell $n ������ ���� �������� ��� ��� ���������, � �� ���� � ���� �� ������.
tell $n �� �� ��� �� ������ ��� ��������� ����?
tell $n ����� ����, ����� ���� ����.
shake $n
endif
endif
~
#54
if ispc $n
if level $n <= 17
smile $n
tell $n �� � �� ����� �������� ... �� ��������� ��������������.
endif
endif
~
#55
if ispc $n
if level $n <= 17
stare
endif
endif
~
#56
if ispc $n
if level $n <= 17
sigh
tell $n �� �������. � ������� ��� ������� � ������ �� ... �� ����� ���.
wave
endif
endif
~
#57
if ispc $n
if level $n <= 17
tell $n ��, ��� ��� ������� ���� ���.
tell $n ������ ��� �����-�� ����. � �������, ��� ���� ���� �������� � ����� ����������, �� � ������.
blush
tell $n ����, �� ��� �� �����.
give key $n
wave $n
endif
endif
~
#58
if ispc $n
if level $n <= 17
smile
say ������� ����, �����������.
say � ��� ������� ������� �� ��, ��� ����� ���� ������� �������.
say ����� �� ���� ���� �������� �������.
mob oload 30163
give blue $n
shake $n
endif
endif
~
#66
if ispc $n
if level $n <= 17
say {c�������, �������.. �����, ��������.. ��� � �� �������� ��-����..
say �-�������, ��������, �������.. 
say {c������, ���� ��������� ��� ���� ��-���� - �� � ����� �� ��������!..{x
endif
endif
~
#67
say {G�! {W�{c�{W�{c�{W��{c�{Y!{x
say {Y������ {c������{Y!{x
say {G�������, �����.. $n, �� ��� ������ �������� �����!{x
emote ���������
dri hemlock
mob junk hemlock
echo {y���� ������{G �������{x
say {C����!{W ����� �� ����!{x
mob cast fly
mob cast fly $n
~
#1001
if ispc $n
if sex $n > 1
say {C��������� {Y������� ������ �������.{x
endif
if carries $n 2423
mob force $n drop diamond
mob force $n sac diamond
mob oload 12453 15 'R'
say {Y����� ������.{x
mob force $n take ring
endif
~
#1002
if ispc $n
if carries $n 6519
mob force $n give pills nichole
mob junk pills
mob oload 3377 15 'R'
emote ������ {C����������� {W�������.{x
endif
endif
~
#1003
if ispc $n
if carries $n 29010
else
if rand 30
say ����� �� �������� $n.
say �� ������� ������ ������� �� ������� ���� � ��������?
say �������� ��������.
say ��� ��� ����� ������� ���� �� ���������� ����� �����.
say � ������ ���� ��������� � ��� � �������, �� � �� ������ ���.
say � ����� ������ � ������ ��������� ������.
endif
endif
endif
~
#1004
if ispc $n
mob junk axe
say ��� ����� ��������, ��� �� ���� ������� �����. � ������ ������.
mob oload 77 1
give metka $n
endif
~
#1006
if ispc $n
if carries $n 77
mob force $n give metka thief
say � �� � ���� �� ���� �� �����.
say ������� ����� �� ����� �� ���������� ��� ����, ��� ����.
say � �������� ������� ����� ����� �� ��������� ������ ����������� {G�{g�{G�{g�{G�{g�{G�.{x
say ������ �� ��������� �����.
mob junk metka
say ����� ����� ����� ���� ��� ���������� ������� ����������� ������� ������.
say ����� ��� � ����� ����� �� ���������� �������.
endif
endif
~
#1007
if ispc $n
if carries $n 16071
mob oload 5099 60 'R'
mob zecho "{Y�� ����������� ������ ����� ������������ ������� �������� � {G�{g�{G�{g�{G�{g�{x"
endif
endif
~
#1008
if ispc $n
if carries $n 30386
mob force $n give eye mole
mob junk eye
mob zecho �� �������� ����� �����.
say ������� ���� �������� �����.
mob oload 11709 1 'R'
say ��� � ��� ����� ���� ������, ����� ���� ����������.
else
say �������� ������ ������ ��� ����� ������� ������, � �� ������ ������ �� ����.
say � ������ � ������� ������ ���� ������, ������� ����� ��������.
say �� � � ����� �� ��������
endif
endif
~
#1009
if ispc $n
if mobhere 3090
say � �� ������ ��� ��������.
say ������ ��� ����� � ��� ������!
emote {B�{W�{B�{W�{B�{W�{B�.{x
mob force $n stopfol
mob purge kitten
mob oload 12012 54 'R'
mob zecho "{W��� ������� ����� {Y������{x"
else
say ��� �� ��� ������� ��� {Y������{x.
say ��� ������� � ��������� ��������.
say ������� �� �����, ������� �� ���, �������� �� ��.
endif
endif
~
#1010
if ispc $n
if carries $n 12012
say � �� ������ ��!
mob force $n drop book
emote ����������� � ������.
mob oload 2235 1 'R'
mob purge book
else
say ������� ��� ��������.
say �� �������� �� �� �� ����� ���� ������ � ������ ����?
say � ����� �� ����� � ����� ������� � ���� ������� ����� �����?
say ��� ��� � �������� �����.
say �� ���� ���� �� ������������ ������� ��� ����!
say ����� �������� ��� �������? � � ���� ������ ��� ������ �� {Y�{W��{Y�{W�{Y�{W�{Y�{W�{Y�{W�{Y�.{x
endif
endif
~
#1011
if ispc $n
if carries $n 2235
say �� ����� ��! ��� �������!
say ����� �� ������ ����.
mob remove $n 2235
mob oload 23056 1 'R'
say ��� ����� ������.
else
say ���� ���, ���� ���!
emote ���� �� ���� ������.
say �������! ����� �������!
say ����� � ���� ��� ��� ��� � ���� ���� ��������� ������?
say �������!
endif
endif
~
#1012
if ispc $n
if isevil $n
if carries $n 14019
mob remove $n 14019
say � �� ���� ���! �������!
say ������ ����� �� ����� �������� {D�{x�{D�{x�!{x
mob oload 20031 1 'R'
else
say � ���� ���� ����� ����������� {D����.{x
say ����� �� �� ��������� ��������� �� ������������� ����� - {W��������?{x
endif
endif
endif
~
#1013
if ispc $n
if isgood $n
if carries $n 20024
mob remove $n 20024
mob cast 'shield' $n
mob cast 'armor' $n
say ����� ����� {W����!{x
else
say � ���� ���� ����� ����������� �����.
say ������ ��� ���������� �� ������� ��� - {D������� �����.{x
endif
endif
endif
~
#1014
if ispc $n
if carries $n 12452
if carries $n 12453
mob remove $n 12453
mob remove $n 12452
say ������� ������!
say ������� ���� � ������!
if wears $n 12429
mob oload 12430
else
mob oload 12429
endif
give straps $n
endif
else
say ������!
say ��� ��������������� ������?
say ���������, ���������.
if rand 50
say � � ���� ��� �������� {Y������������ �������{x?
say ������ ������ �� ����������!
endif
endif
endif
~
#1015
if level $n > 90
mob junk string
endif
~
#1016
if ispc $n
if carries $n 3001
else
say �� ���� ������... ������ ����� ��� �� ����.
say ���� �� "����������" ����� ������� �������.
say ����� �� �������� ���������?
endif
endif
~
#1017
if ispc $n
mob junk bottle
mob gecho ����� ������ �������� ����.
say ������ �����!
mob cast haste $n
endif
~
#1018
if ispc $n
if carries $n 3012
else
say � ������ �� �� �� ������� ��������?
say ��?! ����� �������!
say � �� �� �������� � ����� ������� ���������� �����?
say ����� �������, ��� ���.
endif
endif
~
#1019
if ispc $n
mob junk cinnamon
mob zecho "��������� ������ �������"
say ����������!
mob cast sanctuary $n
endif
~
#1020
if ispc $n
if carries $n 9520
else
say �� � ����� �� ������� ������ ��������� � ����� ������.
say ������� ������� ��������� �� ����.
say ��� ���� ��� � ������ ������.
say � ������ � {C����� ������{G ������� ���� ������.{x
endif
endif
~
#1021
if ispc $n
mob junk shield
mob gecho ��� �������� ������� ���, �� �������� ��� ������� ���� ��� �����.
say ��.... �� ��� �� ����� ���� ���, ��� �����������.
say �� ��� ���� �������.
mob cast shield $n
endif
~
#6700
if ispc $n
if isgood $n
say {W�������, {Y$n{R!!!{x
say {M������, ������ ������ {W������{x
if sex $n < 2
sc
say {W������, �� ���� "{D22{x{W" �����?{x"
mob force $n say {W��, ���� ��!{x
shake $n 
else
sc 
say {W������, �� ���� "{D22{x{W" ��������?{x"
mob force $n say {W�� �� ����! �������-�� ���� � ...{x
kiss $n
endif
else
say {D������ {Y$n{D, � ���� ����� ���.{x
giggle $n
say {M������, ������ ������ {D�������{x
if sex $n < 2
sc
say {W������, �� ���� "{D22{x{W" �����?{x"
mob force $n say {W������� ������ �����!{x
bow $n 
else
sc 
say {W������, �� ���� "{D22{x{W" ��������?{x"
mob force $n say {W�������, �������!{x
smile $n
endif
endif
endif
"
~
#10131
if clan $n 'chaos'
    mob transfer $n 26301
endif
~
#26310
mob echo {x
if clan $n 'chaos'
say ������, ������... �������? �� ��� ������ - ������ ��� �� ������...
mob echo {D��� �������� ����� �����{x ����������� ������ �� �����...
else
say ��� ��� ��� �����?
say � �� ����� ������...
say ����� ����� ������� � ��� ���� �� �����!
mob delay 2
endif
~
#26311
if hastarget $i
growl
say �� ��� �����?!?!
say ��, � ���� ������������...
backstab $q
else
say ���...
mob forget
endif
~
#26320
say ��...
if clan $n 'chaos'
tell $n ������ ������, ���� ���� ��������...
unlock down
open down
mob force $n down
mob at $n emote �������� ����� �� �����.
close down
lock down
end
endif
tell $n �� ���������...
~
#26400
say Hi!
~
#27000
if clan $n 'jumandji'
say ����������, ������! :){x
kiss $n
else       
say {G� ���� �� ��� ������? {R��{G? � �� �����.. ������..{x
murder $n        
say {G��� �� �������? :E{x
endif
~
#27001
if clan $n jumandji
smile $n
nod $n
else
emote ������ �����...
murder $n
grin $n
endif
if name $n 'Dragon'
say '{G��������... ��� ����? ����-�� �����? :){x'
wave $n
mob transfer $n 3054
endif
if name $n 'Astellar'
say '��� ��������, ������...'
emote ���������� ������.
endif
~
#0

#$
