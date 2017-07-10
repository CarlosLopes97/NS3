#!/bin/bash
# VERSAO: 1.1
# NOME:	cenario2.sh
# DESCRICAO: Efetua a criação de video, chamada do simulador NS3 e avaliacao
# de QOe e QOs
# NOTA:	etmp4, ffmpeg, mp4trace, MP4Box e MP4Box requerem o Wine para total funcionamento.
# NOTA 2: configurar o endereço de rede.
# AUTOR: Joahannes Costa <joahannes@gmail.com>.
# EDIT: Leonardo Silva <ti.leonardosilva@gmail.com>.


# ****************** Cenário 2 **************************
# *                                						*
# *   	            -------------UE1  					*
# *      	 		-									*
# *   eNB1------UE0--------------UE2					*
# *      	 		-       							*
# *      	 		-------------UE3					*
# *      	 		-      								*
# *       	 		-------------UE4    				*
# *      	 		-             						*
# *    	     		-            ...					*
# *      	 		-             						*
# *       	 		-------------UEi -> 4/40/400 UES	*
# *														*
# *******************************************************
 

#Variáveis com caminho para diretórios
caminho_video=video
caminho_resul=resultados/cenario2/dados
caminho_graf=resultados/cenario2/graficos
ues=1

#	  ************ Ordem *************
# linha 1 - video 352x288 CIF - bus.yuv  			   '|
# linha 2 - video 640x480 SD - stockholm.yuv 			| fonte: https://media.xiph.org/video/derf/
# linha 3 - video 1280x720 HD - stockholm.yuv 		   .|

sudo ifconfig eth0 192.168.2.1 netmask 255.255.255.0 up

printf "\n"  
echo "------------------- CRIANDO ARQUIVO DO VIDEO -------------------"
printf "\n"
#x264 -I 30 --bframes 2 -B 128 --fps 30 -o $caminho_video/Vp_cif.264 --input-res 352x288 Vi_cif.yuv


#x264 -I 10 --bframes 2 -B 300 --fps 30 -o $caminho_video/Vp_cif.264 --input-res 352x288 Vi_cif.yuv
x264 -I 10 --bframes 2 -B 800 --fps 30 -o $caminho_video/Vp_480p.264 --input-res 640x480 Vi_480p.yuv
#x264 -I 10 --bframes 2 -B 1800 --fps 30 -o $caminho_video/Vp_720p.264 --input-res 1280x720 Vi_720p.yuv

printf "\n"
echo "------------------- CRIAÇÃO DO VÍDEO INICIAL -------------------"
printf "\n"
#MP4Box -hint -mtu 1472 -fps 30 -add $caminho_video/Vp_cif.264 $caminho_video/Vp_cif.mp4


#MP4Box -hint -mtu 1024 -fps 30 -add $caminho_video/Vp_cif.264 $caminho_video/Vp_cif.mp4
MP4Box -hint -mtu 1024 -fps 30 -add $caminho_video/Vp_480p.264 $caminho_video/Vp_480p.mp4
#MP4Box -hint -mtu 1024 -fps 30 -add $caminho_video/Vp_720p.264 $caminho_video/Vp_720p.mp4

printf "\n"
echo "------------------- GERAÇÃO DO ST DO VIDEO INICIAL -------------------"
printf "\n"
#./$caminho_video/mp4trace -f -s 192.168.2.2 12346 1024 $caminho_video/Vp_cif.mp4 > $caminho_video/st_Vi_cif
./$caminho_video/mp4trace -f -s 192.168.2.2 12346 1024 $caminho_video/Vp_480p.mp4 > $caminho_video/st_Vi_480p
#./$caminho_video/mp4trace -f -s 192.168.2.2 12346 1024 $caminho_video/Vp_720p.mp4 > $caminho_video/st_Vi_720p

printf "\n"
echo "------------------- GERANDO YUV ANTES DO ENVIO PARA COMPARAÇÃO -------------------"
printf "\n"
#ffmpeg -i $caminho_video/Vp_cif.mp4 Via_cif.yuv
ffmpeg -i $caminho_video/Vp_480p.mp4 Via_480p.yuv
#ffmpeg -i $caminho_video/Vp_720p.mp4 Via_720p.yuv

printf "\n"
echo "------------------- EXECUÇÃO DA SIMULAÇÃO - NS3 -------------------"
printf "\n"
./waf --run scratch/mcc1 > $caminho_resul/mcc1.log 2>&1

echo "--------------------FIM DA SIMULACAO----------------------------"

printf "\n"
echo "------------------- CRIAÇÃO DO VIDEO RECEBIDO -------------------"
printf "\n"

for ((i=0; i<ues; i++))
do
echo "Vídeo transmitido inicial $i";
#./$caminho_video/etmp4 -F -0 $caminho_resul/sd_lte_$i $caminho_resul/rd_lte_$i $caminho_video/st_Vi_cif $caminho_video/Vp_cif.mp4 Vp_cif$i
./$caminho_video/etmp4 -F -0 $caminho_resul/sd_lte_$i $caminho_resul/rd_lte_$i $caminho_video/st_Vi_480p $caminho_video/Vp_480p.mp4 Vp_480p$i
#./$caminho_video/etmp4 -F -0 $caminho_resul/sd_lte_$i $caminho_resul/rd_lte_$i $caminho_video/st_Vi_720p $caminho_video/Vp_720p.mp4 Vp_720p$i
done

for ((i=0; i<ues; i++))
do
printf "\n"
echo "------------------- GERAÇÃO DO ARQUIVO YUV PARA COMPARAÇÃO -------------------"
printf "\n"

echo "Vídeo transmitido yuv inicial $i";

#ffmpeg -i 'Vp_cif'$i'.mp4' 'Vp_cif'$i'.yuv'
ffmpeg -i 'Vp_480p'$i'.mp4' 'Vp_480p'$i'.yuv'
#ffmpeg -i 'Vp_720p'$i'.mp4' 'Vp_720p'$i'.yuv'

echo "Avaliação SSIM do video inicial $i";

printf "\n"
echo "------------------- AVALIAÇÃO DE QoE - SSIM -------------------"
path=pwd
#wine $caminho_video/msu_metric.exe -f Via_cif.yuv IYUV -yw 352 -yh 288 -f 'Vp_cif'$i'.yuv' -sc 1 -cod $path -metr ssim_precise -cc YYUV
wine $caminho_video/msu_metric.exe -f Via_480p.yuv IYUV -yw 640 -yh 480 -f 'Vp_480p'$i'.yuv' -sc 1 -cod $path -metr ssim_precise -cc YYUV
#wine $caminho_video/msu_metric.exe -f Via_720p.yuv IYUV -yw 1280 -yh 720 -f 'Vp_720p'$i'.yuv' -sc 1 -cod $path -metr ssim_precise -cc YYUV


#psnr 352 288 420 Via_cif.yuv 'Vp_cif'$i'.yuv'


done

rm delay*
rm loss*
rm rate*

printf "\n"
echo "------------------- GERANDO GRAFICOS ----------------------"
printf "\n" 
gnuplot $caminho_graf/lte_FlowVSThroughput.plt
gnuplot $caminho_graf/lte_FlowVSDelay.plt
gnuplot $caminho_graf/lte_FlowVSLostPackets.plt
gnuplot $caminho_graf/lte_FlowVSJitter.plt


#DANDO PERMISSÃO AOS ARQUIVOS CRIADOS
#chmod 777 $caminho_video/Vp_cif.264
#chmod 777 $caminho_video/Vp_cif.mp4
#chmod 777 $caminho_video/st_Vi_cif

chmod 777 $caminho_video/Vi_480p.264
chmod 777 $caminho_video/Vi_480p.mp4
chmod 777 $caminho_video/st_Vi_480p

#chmod 777 $caminho_video/Vp_720p.264
#chmod 777 $caminho_video/Vp_720p.mp4
#chmod 777 $caminho_video/st_Vi_720p

printf "\n"
echo "------------------- FIM DA EXECUÇÃO DO SCRIPT -------------------"
printf "\n"

#EOF