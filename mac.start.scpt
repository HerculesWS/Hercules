FasdUAS 1.101.10   ��   ��    k             l     ��  ��    %  This file is part of Hercules.     � 	 	 >   T h i s   f i l e   i s   p a r t   o f   H e r c u l e s .   
  
 l     ��  ��    = 7 http://herc.ws - http://github.com/HerculesWS/Hercules     �   n   h t t p : / / h e r c . w s   -   h t t p : / / g i t h u b . c o m / H e r c u l e s W S / H e r c u l e s      l     ��������  ��  ��        l     ��  ��    1 + Copyright (C) 2013-2015  Hercules Dev Team     �   V   C o p y r i g h t   ( C )   2 0 1 3 - 2 0 1 5     H e r c u l e s   D e v   T e a m      l     ��������  ��  ��        l     ��  ��    G A Hercules is free software: you can redistribute it and/or modify     �   �   H e r c u l e s   i s   f r e e   s o f t w a r e :   y o u   c a n   r e d i s t r i b u t e   i t   a n d / o r   m o d i f y      l     ��   ��    K E it under the terms of the GNU General Public License as published by      � ! ! �   i t   u n d e r   t h e   t e r m s   o f   t h e   G N U   G e n e r a l   P u b l i c   L i c e n s e   a s   p u b l i s h e d   b y   " # " l     �� $ %��   $ H B the Free Software Foundation, either version 3 of the License, or    % � & & �   t h e   F r e e   S o f t w a r e   F o u n d a t i o n ,   e i t h e r   v e r s i o n   3   o f   t h e   L i c e n s e ,   o r #  ' ( ' l     �� ) *��   ) * $ (at your option) any later version.    * � + + H   ( a t   y o u r   o p t i o n )   a n y   l a t e r   v e r s i o n . (  , - , l     ��������  ��  ��   -  . / . l     �� 0 1��   0 F @ This program is distributed in the hope that it will be useful,    1 � 2 2 �   T h i s   p r o g r a m   i s   d i s t r i b u t e d   i n   t h e   h o p e   t h a t   i t   w i l l   b e   u s e f u l , /  3 4 3 l     �� 5 6��   5 E ? but WITHOUT ANY WARRANTY; without even the implied warranty of    6 � 7 7 ~   b u t   W I T H O U T   A N Y   W A R R A N T Y ;   w i t h o u t   e v e n   t h e   i m p l i e d   w a r r a n t y   o f 4  8 9 8 l     �� : ;��   : D > MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    ; � < < |   M E R C H A N T A B I L I T Y   o r   F I T N E S S   F O R   A   P A R T I C U L A R   P U R P O S E .     S e e   t h e 9  = > = l     �� ? @��   ? 3 - GNU General Public License for more details.    @ � A A Z   G N U   G e n e r a l   P u b l i c   L i c e n s e   f o r   m o r e   d e t a i l s . >  B C B l     ��������  ��  ��   C  D E D l     �� F G��   F H B You should have received a copy of the GNU General Public License    G � H H �   Y o u   s h o u l d   h a v e   r e c e i v e d   a   c o p y   o f   t h e   G N U   G e n e r a l   P u b l i c   L i c e n s e E  I J I l     �� K L��   K L F along with this program.  If not, see <http://www.gnu.org/licenses/>.    L � M M �   a l o n g   w i t h   t h i s   p r o g r a m .     I f   n o t ,   s e e   < h t t p : / / w w w . g n u . o r g / l i c e n s e s / > . J  N O N l     ��������  ��  ��   O  P Q P l     ��������  ��  ��   Q  R S R l     �� T U��   T   Notes    U � V V    N o t e s S  W X W l     �� Y Z��   Y ^ X    although merging the cd and myPath 'do script' lines could be possible, the server's    Z � [ [ �         a l t h o u g h   m e r g i n g   t h e   c d   a n d   m y P a t h   ' d o   s c r i p t '   l i n e s   c o u l d   b e   p o s s i b l e ,   t h e   s e r v e r ' s X  \ ] \ l     �� ^ _��   ^ a [    working directory will not read properly (it'll try to read its files off your '~' dir)    _ � ` ` �         w o r k i n g   d i r e c t o r y   w i l l   n o t   r e a d   p r o p e r l y   ( i t ' l l   t r y   t o   r e a d   i t s   f i l e s   o f f   y o u r   ' ~ '   d i r ) ]  a b a l     ��������  ��  ��   b  c�� c l   
 d���� d O    
 e f e k   	 g g  h i h I   	������
�� .miscactvnull��� ��� null��  ��   i  j k j l  
 
��������  ��  ��   k  l m l l  
 
�� n o��   n 7 1 we strip the file name from the (path to me) var    o � p p b   w e   s t r i p   t h e   f i l e   n a m e   f r o m   t h e   ( p a t h   t o   m e )   v a r m  q r q r   
  s t s m   
  u u � v v " m a c . r a t h e n a - s t a r t t n      w x w 1    ��
�� 
txdl x 1    ��
�� 
ascr r  y z y r     { | { n     } ~ } 1    ��
�� 
strq ~ l    ����  n     � � � 1    ��
�� 
psxp � l    ����� � l    ����� � I   �� ���
�� .earsffdralis        afdr �  f    ��  ��  ��  ��  ��  ��  ��   | o      ���� 0 mypath myPath z  � � � r    $ � � � l   " ����� � e    " � � b    " � � � n      � � � 4    �� �
�� 
citm � m    ����  � l    ����� � o    ���� 0 mypath myPath��  ��   � m     ! � � � � �  '��  ��   � o      ���� 0 mypath myPath �  � � � r   % * � � � m   % & � � � � �   � n      � � � 1   ' )��
�� 
txdl � 1   & '��
�� 
ascr �  � � � l  + +��������  ��  ��   �  � � � O  + C � � � O  / B � � � I  6 A�� � �
�� .prcskprsnull���     ctxt � m   6 7 � � � � �  t � �� ���
�� 
faal � m   : =��
�� eMdsKcmd��   � 4   / 3�� �
�� 
prcs � m   1 2 � � � � �  T e r m i n a l � m   + , � ��                                                                                  sevs  alis    �  MacBook Pro SSD            �֭cH+  ��RSystem Events.app                                              �ʆ����        ����  	                CoreServices    �֟S      ����    ��R��Q��P  @MacBook Pro SSD:System: Library: CoreServices: System Events.app  $  S y s t e m   E v e n t s . a p p     M a c B o o k   P r o   S S D  -System/Library/CoreServices/System Events.app   / ��   �  � � � l  D D��������  ��  ��   �  � � � I  D Z�� � �
�� .coredoscnull��� ��� ctxt � b   D I � � � m   D G � � � � �  c d   � o   G H���� 0 mypath myPath � �� ���
�� 
kfil � n   L V � � � 1   R V��
�� 
tcnt � l  L R ����� � 4  L R�� �
�� 
cwin � m   P Q���� ��  ��  ��   �  � � � I  [ o�� � �
�� .coredoscnull��� ��� ctxt � m   [ ^ � � � � � $ . / l o g i n - s e r v e r _ s q l � �� ���
�� 
kfil � n   a k � � � 1   g k��
�� 
tcnt � l  a g ����� � 4  a g�� �
�� 
cwin � m   e f���� ��  ��  ��   �  � � � l  p p��������  ��  ��   �  � � � O  p � � � � O  t � � � � I  } ��� � �
�� .prcskprsnull���     ctxt � m   } � � � � � �  t � �� ���
�� 
faal � m   � ���
�� eMdsKcmd��   � 4   t z�� �
�� 
prcs � m   v y � � � � �  T e r m i n a l � m   p q � ��                                                                                  sevs  alis    �  MacBook Pro SSD            �֭cH+  ��RSystem Events.app                                              �ʆ����        ����  	                CoreServices    �֟S      ����    ��R��Q��P  @MacBook Pro SSD:System: Library: CoreServices: System Events.app  $  S y s t e m   E v e n t s . a p p     M a c B o o k   P r o   S S D  -System/Library/CoreServices/System Events.app   / ��   �  � � � l  � ���������  ��  ��   �  � � � I  � ��� � �
�� .coredoscnull��� ��� ctxt � b   � � � � � m   � � � � � � �  c d   � o   � ����� 0 mypath myPath � �� ���
�� 
kfil � n   � � � � � 1   � ���
�� 
tcnt � l  � � ����� � 4  � ��� �
�� 
cwin � m   � ����� ��  ��  ��   �  � � � I  � ��� � �
�� .coredoscnull��� ��� ctxt � m   � � � � � � � " . / c h a r - s e r v e r _ s q l � �� ���
�� 
kfil � n   � � � � � 1   � ���
�� 
tcnt � l  � � ����� � 4  � ��� �
�� 
cwin � m   � ����� ��  ��  ��   �  � � � I  � ��� ��
�� .sysodelanull��� ��� nmbr � m   � ��~�~ �   �  � � � O  � � � � � O  � � � � � I  � ��} � �
�} .prcskprsnull���     ctxt � m   � � � � � � �  t � �| ��{
�| 
faal � m   � ��z
�z eMdsKcmd�{   � 4   � ��y �
�y 
prcs � m   � �   �  T e r m i n a l � m   � ��                                                                                  sevs  alis    �  MacBook Pro SSD            �֭cH+  ��RSystem Events.app                                              �ʆ����        ����  	                CoreServices    �֟S      ����    ��R��Q��P  @MacBook Pro SSD:System: Library: CoreServices: System Events.app  $  S y s t e m   E v e n t s . a p p     M a c B o o k   P r o   S S D  -System/Library/CoreServices/System Events.app   / ��   �  l  � ��x�w�v�x  �w  �v    I  � ��u
�u .coredoscnull��� ��� ctxt b   � �	
	 m   � � �  c d  
 o   � ��t�t 0 mypath myPath �s�r
�s 
kfil n   � � 1   � ��q
�q 
tcnt l  � ��p�o 4  � ��n
�n 
cwin m   � ��m�m �p  �o  �r    I  ��l
�l .coredoscnull��� ��� ctxt m   � � �   . / m a p - s e r v e r _ s q l �k�j
�k 
kfil n   � 1   ��i
�i 
tcnt l  � ��h�g 4  � ��f
�f 
cwin m   � ��e�e �h  �g  �j   �d l �c�b�a�c  �b  �a  �d   f m     �                                                                                      @ alis    v  MacBook Pro SSD            �֭cH+  �ccTerminal.app                                                   ������B        ����  	                	Utilities     �֟S      ���"    �cc��q  5MacBook Pro SSD:Applications: Utilities: Terminal.app     T e r m i n a l . a p p     M a c B o o k   P r o   S S D  #Applications/Utilities/Terminal.app   / ��  ��  ��  ��       �` �`   �_
�_ .aevtoappnull  �   � ****  �^!�]�\"#�[
�^ .aevtoappnull  �   � ****! k    
$$  c�Z�Z  �]  �\  "  # "�Y u�X�W�V�U�T�S�R � � ��Q � ��P�O�N ��M�L�K�J � � � � ��I  �
�Y .miscactvnull��� ��� null
�X 
ascr
�W 
txdl
�V .earsffdralis        afdr
�U 
psxp
�T 
strq�S 0 mypath myPath
�R 
citm
�Q 
prcs
�P 
faal
�O eMdsKcmd
�N .prcskprsnull���     ctxt
�M 
kfil
�L 
cwin
�K 
tcnt
�J .coredoscnull��� ��� ctxt
�I .sysodelanull��� ��� nmbr�[�*j O���,FO)j �,�,E�O��k/�%E�O���,FO� *��/ �a a l UUOa �%a *a k/a ,l Oa a *a k/a ,l O� *�a / a a a l UUOa �%a *a k/a ,l Oa a *a k/a ,l Omj O� *�a / a a a l UUOa  �%a *a k/a ,l Oa !a *a k/a ,l OPUascr  ��ޭ