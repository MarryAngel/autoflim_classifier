#!/bin/bash

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"

compile_dir () {
    DIR="$1"

    echo "===================================="
    echo "Entrando em: $DIR"
    echo "===================================="

    cd "$ROOT_DIR/$DIR" || {
        echo "❌ Erro: diretório não encontrado: $DIR"
        exit 1
    }

    sh compile.sh 1

    if [ $? -eq 0 ]; then
        echo "✅ Compilação concluída com sucesso em $DIR"
    else
        echo "❌ Erro ao compilar em $DIR"
        exit 2
    fi
}

case "$1" in
  1) compile_dir "ift/demo/ImageGraph" ;;
  2) compile_dir "ift/demo/FLIM/FilterEstimationMethods" ;;
  3) compile_dir "ift/demo/Miscellaneous/ImageConverters" ;;
  4) compile_dir "ift/demo/FLIM/BoFP" ;;
  5) compile_dir "ift/demo/Classification" ;;
  6) compile_dir "ift/demo/ObjBasedSuperSegm" ;;
  all)
    for i in {1..6}; do
      bash "$0" $i
    done
    ;;
  *)
    echo "Uso: sh compile.sh <número>"
    echo "1  ift/demo/ImageGraph"
    echo "2  ift/demo/FLIM/FilterEstimationMethods"
    echo "3  ift/demo/Miscellaneous/ImageConverters"
    echo "4  ift/demo/FLIM/BoFP"
    echo "5  ift/demo/Classification"
    echo "6  ift/demo/ObjBasedSuperSegm"
    exit 3
    ;;
esac

