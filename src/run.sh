#!/bin/bash
EXEC="./ted"
BASE="../testes"
SAIDA="../saida"

CONJUNTOS=("c1" "c2" "c3")

QRYS=(
    "censo"
    "despj-001"
    "despj-001-hab"
    "despj-010"
    "despj-100"
    "despj-100-hab"
    "falecimento-unico-morador"
    "falecimento-unico-st"
    "falecimentos-30-espalhados-moradores"
    "falecimentos-30-espalhados-st"
    "falecimentos-30-espalhados-st-censo"
    "mudanca-todos-moradores-com-verif"
    "nasc-001"
    "nasc-001-hab"
    "nasc-010"
    "nasc-100"
    "nasc-100-hab"
    "remocao-1-quadra-com-morad"
    "remocao-centro"
)

for C in "${CONJUNTOS[@]}"; do
    echo "=============================="
    echo " Conjunto: $C"
    echo "=============================="

    GEO_PATH="$BASE/$C.geo"
    PM_PATH="$BASE/$C.pm"

    if [ ! -f "$GEO_PATH" ]; then
        echo "  [SKIP] $C.geo não encontrado"
        continue
    fi
    if [ ! -f "$PM_PATH" ]; then
        echo "  [SKIP] $C.pm não encontrado"
        continue
    fi

    OUT="$SAIDA/$C"
    mkdir -p "$OUT"

    for QRY in "${QRYS[@]}"; do
        QRY_PATH="$BASE/$C/$QRY.qry"
        if [ ! -f "$QRY_PATH" ]; then
            echo "  [SKIP] $QRY.qry não encontrado"
            continue
        fi
        echo -n "  Rodando $QRY... "
        $EXEC -e "$BASE" -f "$C.geo" -pm "$C.pm" -o "$OUT" -q "$C/$QRY.qry" 2>/dev/null
        RET=$?
        if [ $RET -eq 0 ]; then
            echo "[OK]"
        else
            echo "FALHOU (exit code $RET)"
        fi
    done
    echo ""
done

echo "Testes concluídos. Saídas em $SAIDA/"