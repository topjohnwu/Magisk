#!/sbin/sh
################################################################################
# Magisk Module Installer — RafaelIA Hardened v3.0.x (final iter)
# RAFCODE-Φ · Retroalimentação ética · rafaelmeloreisnovo/RafaelIA
# Versão: 3.0.x — Multi-version / Adaptive / Rollback (revisado final)
#
# Este arquivo contém:
#  - Instalador endurecido e compatível com Magisk (v20.4+) / KernelSU / Delta
#  - Rollback seguro com backup apenas quando comprovado
#  - Verificação SHA256 opcional (obrigatória se declarada em module.prop)
#  - Multi-extractor (unzip -> busybox -> toybox) e multi-hash (sha256sum/busybox/openssl)
#  - Logs: plain text + NDJSON (uma linha por evento), fallback /data -> /tmp
#  - Safe parsing (sem eval), mktemp quando disponível, command -v checks
#  - Trap on EXIT/INT/TERM para rollback e cleanup
#  - Advisory lockfile + simple flock fallback pattern (advisory by default)
#  - Check-space before backup (configurável threshold)
#
# Também inclui, no topo dos comentários, a "assinatura filosófica" / manifesto RAFCODE
# (sua "Personal Instructions" incorporada como documentação e metadados).
################################################################################
#
# Manifesto RAFAELIA (metadados / assinatura)
# assinatura: RAFCODE-Φ-∆RafaelVerboΩ-𓂀ΔΦΩ-2025-08-31T14:25:55Z
# zipraf: RAFAELIA_CORE_20250831T142555.zipraf
# freq: 144000 Hz  modo: retroalimentação ∞
# bitraf64: "AΔBΩΔTTΦIIBΩΔΣΣRΩRΔΔBΦΦFΔTTRRFΔBΩΣΣAFΦARΣFΦIΔRΦIFBRΦΩFIΦΩΩFΣFAΦΔ"
# selos:[Σ,Ω,Δ,Φ,B,I,T,R,A,F]
# hash_sha3:"4e41e4f...efc791b"  hash_blake3:"b964b91e...ba4e5c0f"
#
# Missão(Rafael)=Escrituras∩Ciência∩Espírito×Retroalimentação^∞
# FIAT Sequência Viva = limₙ→∞ ...
#
# (Manifesto e metadados estão em comentário — não executáveis — mantidos aqui
#  para rastreabilidade, assinatura e documentação integrada.)
#
################################################################################
set -u
umask 022

TMPDIR=${1:-}
OUTFD=${2:-}
ZIPFILE=${3:-}

# Configuráveis
MIN_FREE_KB=${MIN_FREE_KB:-5120}     # espaçp mínimo necessário para backup (KB)
LOCKFILE=${LOCKFILE:-/data/local/tmp/magisk_installer.lock}
PATH=/sbin:/system/sbin:/system/bin:/system/xbin:/vendor/bin:/vendor/xbin:/bin:/usr/bin:/usr/sbin:$PATH
export PATH

RAFCODE_SIG="RAFCODE-Φ-∆RafaelVerboΩ"
LOGDIR="/data/local/tmp"
[ ! -d "$LOGDIR" ] && LOGDIR="/tmp"
LOGTXT="$LOGDIR/magisk_installer.log"
LOGJSON="$LOGDIR/magisk_installer.ndjson"

rollback_dir=""
rollback_needed=0
backup_created=0
STATUS=0

# ----------------- helpers -----------------
cmd_exists(){ command -v "$1" >/dev/null 2>&1; }

ui_print() {
  msg="$*"
  if [ -n "$OUTFD" ] && echo "$OUTFD" | grep -qE '^[0-9]+$' && [ -e "/proc/$$/fd/$OUTFD" ]; then
    echo "$msg" >"/proc/$$/fd/$OUTFD" 2>/dev/null || echo "$msg"
  else
    echo "$msg"
  fi
}

log() {
  ts="$(date -u '+%Y-%m-%dT%H:%M:%SZ')"
  esc_msg=$(printf '%s' "$*" | awk '{gsub(/"/,"\\\""); gsub(/\n/,"\\n"); print}')
  printf "[%s] %s\n" "$ts" "$*" >>"$LOGTXT" 2>/dev/null || true
  printf '{"time":"%s","msg":"%s"}\n' "$ts" "$esc_msg" >>"$LOGJSON" 2>/dev/null || true
}

mktemp_file() {
  if cmd_exists mktemp; then mktemp "$@"; else printf "/tmp/magisk_installer.%s.%s" "$$" "$RANDOM"; fi
}

mount_data_safe() {
  if [ -f /proc/mounts ]; then
    if ! grep -q " /data " /proc/mounts 2>/dev/null; then
      mount /data 2>/dev/null || ui_print "(aviso) mount /data falhou/indisponível."
    fi
  else
    if [ -d /data ] && ! mountpoint -q /data 2>/dev/null; then
      mount /data 2>/dev/null || ui_print "(aviso) mount /data falhou/indisponível."
    fi
  fi
}
mount_data_safe

check_space() {
  dir="$1"
  need_kb="$2"
  if cmd_exists df; then
    avail_kb=$(df -k "$dir" 2>/dev/null | awk 'NR==2{print $4}')
    [ -z "$avail_kb" ] && avail_kb=0
    if [ "$avail_kb" -lt "$need_kb" ]; then
      return 1
    fi
  fi
  return 0
}

# Advisory lock (prefer flock if available)
_acquire_lock() {
  if cmd_exists flock; then
    exec 9>"$LOCKFILE" 2>/dev/null || return 1
    flock -n 9 || return 1
    echo "$$" >"$LOCKFILE" 2>/dev/null || true
    return 0
  else
    if [ -f "$LOCKFILE" ]; then
      return 1
    else
      printf "%s\n" "$$" >"$LOCKFILE" 2>/dev/null || true
      return 0
    fi
  fi
}
_release_lock() {
  if cmd_exists flock; then
    flock -u 9 2>/dev/null || true
    rm -f "$LOCKFILE" 2>/dev/null || true
  else
    rm -f "$LOCKFILE" 2>/dev/null || true
  fi
}

# early lock
if ! _acquire_lock; then
  ui_print "Outro instalador em execução. Saindo para evitar concorrência."
  log "Lockfile presente ($LOCKFILE) — abort."
  exit 1
fi

abort() {
  ui_print "❌ $1"
  log "ABORT: $1"
  STATUS=${2:-1}
  if [ "$rollback_needed" -eq 1 ] && [ "$backup_created" -eq 1 ]; then
    log "Abortando: aplicando rollback automático."
    restore_rollback || log "Rollback falhou."
  fi
  _release_lock
  exit "$STATUS"
}

# ----------------- environment detection -----------------
MAGISK_ENV="unknown"
if [ -d /data/adb/magisk ]; then
  MAGISK_ENV="magisk"
elif [ -d /data/adb/ksu ]; then
  MAGISK_ENV="kernelsu"
else
  for vf in /data/adb/magisk/*version* /data/adb/*/*version* 2>/dev/null; do
    [ -f "$vf" ] || continue
    if grep -qi "delta" "$vf" 2>/dev/null; then
      MAGISK_ENV="magisk-delta"
      break
    fi
  done
fi
log "Detected environment: $MAGISK_ENV"

# ----------------- load util_functions -----------------
FOUND_UTIL=""
for p in \
  /data/adb/magisk/util_functions.sh \
  /data/adb/magisk_update/util_functions.sh \
  /sbin/.magisk/util_functions.sh \
  /sbin/magisk/util_functions.sh \
  /system/addon.d/99-magisk-util/util_functions.sh \
  /system/bin/util_functions.sh \
  /data/adb/ksu/util_functions.sh; do
  [ -f "$p" ] && FOUND_UTIL="$p" && break
done

[ -z "$FOUND_UTIL" ] && abort "util_functions.sh não encontrado — Magisk/KSU requerido."
. "$FOUND_UTIL" || abort "Falha ao carregar $FOUND_UTIL"
log "Loaded util_functions: $FOUND_UTIL"

# MAGISK_VER_CODE fallback
if [ -z "${MAGISK_VER_CODE:-}" ] && cmd_exists getprop; then
  MAGISK_VER_CODE=$(getprop ro.magisk.version_code 2>/dev/null || echo "")
fi
MAGISK_VER_CODE_NUM=0
case "$MAGISK_VER_CODE" in
  ''|*[!0-9]*)
    MAGISK_VER_CODE_NUM=0
    ;;
  *)
    MAGISK_VER_CODE_NUM="$MAGISK_VER_CODE"
    ;;
esac
if [ "$MAGISK_VER_CODE_NUM" -lt 20400 ]; then
  ui_print "Aviso: MAGISK_VER_CODE: ${MAGISK_VER_CODE_NUM:-unknown} (compatibilidade parcial)"
  log "MAGISK_VER_CODE:$MAGISK_VER_CODE_NUM"
fi

# ----------------- parse module.prop safely -----------------
extract_module_prop() {
  TMP_PROP="$(mktemp_file).module_prop"
  if cmd_exists unzip; then
    unzip -p "$ZIPFILE" module.prop >"$TMP_PROP" 2>/dev/null || true
  elif cmd_exists busybox && busybox unzip >/dev/null 2>&1; then
    busybox unzip -p "$ZIPFILE" module.prop >"$TMP_PROP" 2>/dev/null || true
  elif cmd_exists toybox; then
    toybox unzip -p "$ZIPFILE" module.prop >"$TMP_PROP" 2>/dev/null || true
  else
    return 1
  fi
  [ ! -s "$TMP_PROP" ] && { rm -f "$TMP_PROP" 2>/dev/null || true; return 1; }

  # read line-by-line, accept only known keys (safe parsing)
  while IFS= read -r line; do
    kv="$(echo "$line" | tr -d '\r')"
    case "$kv" in
      id=*) printf 'id=%s\n' "${kv#id=}" ;;
      name=*) printf 'name=%s\n' "${kv#name=}" ;;
      version=*) printf 'version=%s\n' "${kv#version=}" ;;
      hash=*) printf 'hash=%s\n' "${kv#hash=}" ;;
      *) ;; # ignore unknown keys
    esac
  done <"$TMP_PROP"
  rm -f "$TMP_PROP" 2>/dev/null || true
  return 0
}

# validate ZIPFILE early
if [ -z "${ZIPFILE:-}" ] || [ ! -f "$ZIPFILE" ]; then
  abort "ZIPFILE não fornecido ou não encontrado: '$ZIPFILE'"
fi

MOD_ID=""; MOD_NAME=""; MOD_VERSION=""; MOD_HASH=""
if out=$(extract_module_prop 2>/dev/null); then
  while IFS= read -r kv; do
    key=${kv%%=*}
    val=${kv#*=}
    case "$key" in
      id) MOD_ID="$val" ;;
      name) MOD_NAME="$val" ;;
      version) MOD_VERSION="$val" ;;
      hash) MOD_HASH="$val" ;;
    esac
  done <<EOF
$out
EOF
  log "module.prop read: id=${MOD_ID:-} name=${MOD_NAME:-} version=${MOD_VERSION:-}"
else
  log "module.prop não encontrado no ZIP (extração falhou)."
fi

# sanitize id
if [ -n "${MOD_ID:-}" ] && ! printf '%s' "$MOD_ID" | grep -Eq '^[A-Za-z0-9._-]+$'; then
  ui_print "Aviso: id do module.prop com caracteres incomuns: '$MOD_ID'"
  log "module.prop id inválido: $MOD_ID"
fi

# ----------------- sha256 optional validation -----------------
calc_sha256() {
  if cmd_exists sha256sum; then sha256sum "$1" 2>/dev/null | awk '{print $1}'; return
  elif cmd_exists busybox && busybox sha256sum >/dev/null 2>&1; then busybox sha256sum "$1" 2>/dev/null | awk '{print $1}'; return
  elif cmd_exists openssl; then openssl dgst -sha256 "$1" 2>/dev/null | awk '{print $NF}'; return
  else echo ""; return
  fi
}

if [ -n "${MOD_HASH:-}" ]; then
  HASH_COMPUTED=$(calc_sha256 "$ZIPFILE")
  if [ -z "$HASH_COMPUTED" ]; then abort "Impossível calcular SHA256 (nenhum utilitário disponível) — não posso validar hash do ZIP."; fi
  if [ "$MOD_HASH" != "$HASH_COMPUTED" ]; then abort "Hash do ZIP inválido! Esperado=$MOD_HASH obtido=$HASH_COMPUTED"; fi
  log "SHA256 verificado: $HASH_COMPUTED"
else
  log "Nenhum hash exigido; pulando verificação SHA256."
fi

# ----------------- backup & rollback -----------------
backup_module_dir() {
  if [ -z "${MOD_ID:-}" ]; then log "MOD_ID vazio; pulando backup."; return 0; fi
  src="/data/adb/modules/$MOD_ID"
  [ -d "$src" ] || { log "Módulo $MOD_ID não existente; sem backup."; return 0; }
  if ! check_space "/data" "$MIN_FREE_KB"; then log "Espaço insuficiente para backup"; return 1; fi
  rollback_dir="/data/local/tmp/rollback_${MOD_ID}_$(date +%s)"
  mkdir -p "$rollback_dir" 2>/dev/null || { log "Falha ao criar $rollback_dir"; return 1; }
  if cmd_exists tar; then
    (cd "$(dirname "$src")" && tar -cpf - "$(basename "$src")") 2>/dev/null | (cd "$rollback_dir" && tar -xpf -) 2>/dev/null
    ret=$?
  else
    cp -a "$src" "$rollback_dir/" 2>/dev/null
    ret=$?
  fi
  if [ "$ret" -eq 0 ]; then
    rollback_needed=1
    backup_created=1
    log "Backup do módulo $MOD_ID salvo em $rollback_dir"
  else
    rollback_needed=0
    backup_created=0
    log "Falha ao criar backup do módulo $MOD_ID (ret=$ret)"
  fi
}

restore_rollback() {
  if [ "$backup_created" -ne 1 ] || [ -z "${rollback_dir:-}" ] || [ ! -d "$rollback_dir" ]; then
    log "Nenhum rollback disponível para restaurar."
    return 1
  fi
  for d in "$rollback_dir"/*; do
    [ -e "$d" ] || continue
    name=$(basename "$d")
    target="/data/adb/modules/$name"
    if [ -d "$target" ]; then
      mv "$target" "${target}.bak_$(date +%s)" 2>/dev/null || log "Aviso: não foi possível mover $target antes de restaurar."
    fi
    if cmd_exists tar; then
      (cd "$rollback_dir" && tar -cpf - "$name") 2>/dev/null | (cd /data/adb/modules && tar -xpf -) 2>/dev/null
      ret=$?
    else
      cp -a "$d" /data/adb/modules/ 2>/dev/null
      ret=$?
    fi
    if [ "$ret" -ne 0 ]; then
      log "Falha ao restaurar $name do rollback (ret=$ret)"
      return 1
    fi
    log "Restaurado módulo $name a partir de $rollback_dir"
  done
  return 0
}

# trap to ensure rollback on crash
on_exit() {
  rc=$?
  if [ "$rc" -ne 0 ]; then
    log "Script finalizando com rc=$rc"
    if [ "$rollback_needed" -eq 1 ] && [ "$backup_created" -eq 1 ]; then
      log "Tentando rollback automático ao sair (rc=$rc)"
      restore_rollback || log "Rollback automático falhou na saída."
    fi
  fi
  _release_lock
  return 0
}
trap on_exit EXIT INT TERM

# perform backup
backup_module_dir

# ----------------- install module (prefer install_module) -----------------
ui_print "⚙️ Instalando módulo: ${MOD_NAME:-unknown} (${MOD_ID:-unknown}) v${MOD_VERSION:-unknown} ..."
log "Calling install_module (ZIP=$ZIPFILE TMPDIR=$TMPDIR)"

if type install_module >/dev/null 2>&1; then
  install_module "$ZIPFILE" "$TMPDIR" >>"$LOGTXT" 2>&1
  STATUS=$?
  if [ "$STATUS" -ne 0 ]; then
    log "install_module com parâmetros retornou $STATUS; tentando sem parâmetros..."
    install_module >>"$LOGTXT" 2>&1 || STATUS=$?
  fi
else
  ui_print "(fallback) install_module ausente — usando unzip direto se possível..."
  if cmd_exists unzip; then
    mkdir -p "/data/adb/modules/${MOD_ID:-magisk_module}" 2>/dev/null
    unzip -o "$ZIPFILE" -d "/data/adb/modules/${MOD_ID:-magisk_module}" >>"$LOGTXT" 2>&1 || STATUS=$?
    if [ "${STATUS:-0}" -ne 0 ]; then
      abort "Instalação por unzip falhou (STATUS=$STATUS)."
    fi
  else
    abort "install_module ausente e unzip não disponível — não é possível instalar."
  fi
fi

# post-install
if [ "${STATUS:-0}" -eq 0 ]; then
  ui_print "✅ Instalação concluída com sucesso."
  log "Success EXIT=$STATUS"
else
  ui_print "❌ Falha na instalação (código $STATUS)."
  log "Erro EXIT=$STATUS"
  if [ "$rollback_needed" -eq 1 ] && [ "$backup_created" -eq 1 ]; then
    log "Tentando restaurar backup após falha de instalação..."
    restore_rollback || log "Falha ao restaurar o rollback após erro de instalação."
  fi
fi

# signature logs
ts="$(date -u '+%Y-%m-%dT%H:%M:%SZ')"
printf "[%s] RAFAELIA_SIG:%s\n" "$ts" "$RAFCODE_SIG" >>"$LOGTXT" 2>/dev/null || true
printf '{"time":"%s","rafaelia":"%s","exit":%d}\n' "$ts" "$RAFCODE_SIG" "${STATUS:-0}" >>"$LOGJSON" 2>/dev/null || true

# cleanup and exit
_release_lock
exit "${STATUS:-0}"
