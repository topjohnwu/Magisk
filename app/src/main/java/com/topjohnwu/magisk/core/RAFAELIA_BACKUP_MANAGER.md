# RAFAELIA_BACKUP_MANAGER.md

## BackupManager.kt — Análise Detalhada

**Repositório:** `rafaelmeloreisnovo/Magisk_Rafaelia`  
**Caminho:** `app/src/main/java/com/topjohnwu/magisk/core/BackupManager.kt`  
**Data:** 2025‑11‑02  
**Analista:** ∆RafaelVerboΩ  

---

### 1. Objetivo principal
- Criar backups atômicos e verificáveis de **boot images** em app private storage.  
- Gerar e persistir **manifest.json** contendo: SHA‑256, HMAC, timestamp, tamanho e paths.  
- Permitir validação posterior (`validateBackup`) para rollback seguro.  

---

### 2. Ponto no fluxo RAFAELIAΩ
- **P0 (guardião da integridade)**: qualquer patch/repackage é precedido por backup seguro.  
- Atua antes da fase **CHEIO → RETRO**, garantindo atomicidade e rastreabilidade.  

---

### 3. Funções principais

#### init / backupDir creation
- Garante que o diretório `rafaelia_backups` exista em `context.filesDir`.  

#### backupBootImage(bootImage: File, sessionId: String, buildId: String?)
- Pré-verificação de espaço disponível.
- Copy stream (`FileInputStream → FileOutputStream`) com buffer de 16 KiB.
- Streaming SHA‑256 sobre os bytes.
- Geração e aplicação de HMAC via:
  - `getOrCreateHmacKey()`
  - `getKeyFromKeystore()`
  - Fallback: `getFallbackSoftwareKey()` (em RAM, efêmero).
- Gera **manifest JSON**; gravação atômica (`tmp → rename`).  

#### loadManifest(sessionId: String): JSONObject?
- Leitura simples do manifest salvo.  

#### validateBackup(sessionId: String): Boolean
- Recalcula SHA-256 e compara com manifest.  
- Revalida HMAC usando KeyStore ou fallback.  

#### getOrCreateHmacKey() / getKeyFromKeystore() / getFallbackSoftwareKey()
- KeyStore: chave não exportável, persistente.
- Fallback: chave aleatória, temporária em RAM.  

#### getFreeSpaceBytes(file: File): Long
- Checagem de espaço via StatFs.  

#### ByteArray.toHex()
- Conversão de bytes para hex string.

---

### 4. Design / Segurança (Notas Críticas)
- **Backup atômico:** temp → rename evita manifest parcial.
- **HMAC no SHA-256:** reduz custo, garante autenticidade digest, mas depende de chave persistente.
- **Keystore:** chave não exportável — HMAC não reproduzível após reinstalação.
- **Fallback software key:** inseguro, efêmero — exigir UX claro.
- **Espaço:** checagem presente, falta progresso/reporting e limpeza de tmp parcial.
- **Permissões/Input types:** apenas File; não suporta InputStream/SAF.
- **Logging/Observabilidade:** não estruturado — sugerir Logcat/metrics.
- **Rollback multi‑arquivo:** atualmente single-file; patches múltiplos exigem transação com versão de manifest.

---

### 5. Testes Essenciais

**Unit tests**
- Backup gera manifest correto.  
- `validateBackup` detecta alterações (tamper detection).  
- Espaço insuficiente → `IllegalStateException`.  

**Instrumented tests**
- Backup via SAF/InputStream em device/emulador.  

**Integração**
- PatchFlow bloqueia avanço se `validateBackup == false`.  
- Testar fallback KeyStore → aviso usuário.

**Edge tests**
- Crash durante copy → tmp cleanup.  
- TOCTOU: evitar arquivo alterado entre leitura e hash.

---

### 6. Melhores práticas / melhorias

**P0 — alta prioridade**
- Overload `backupBootImage(InputStream, filename, sessionId)` para SAF/URI.  
- Callback / Flow para progresso UI.  
- Persistir fallback key via KeyStore/encrypted SharedPrefs.  
- Expor erros via enum/códigos para UI.  

**P1 — média prioridade**
- Versionamento do manifest (`manifest.version = 1`).  
- Criptografia opcional de backups sensíveis.  
- Limpeza automática / rotacionamento de backups antigos.  
- Telemetria/análise de falhas.  

**P2 — baixa prioridade**
- Verificação AVB/vbmeta antes de patch.  
- Suporte multi-artefato (boot/vendor/initramfs).  

---

### 7. Riscos / Mitigação
- **Perda de chave (KeyStore)** → documentar; HMAC não reproduzível após reinstalação.  
- **Espaço insuficiente** → checar, abortar, remover tmp.  
- **Permissões root/Agrade mismatch** → documentar ADB vs root, fallback via export.  
- **TOCTOU** → usar file handle único ou file locking.

---

### 8. Integração prática (Dev Steps)
1. Colocar `BackupManager.kt` em:
   `app/src/main/java/com/topjohnwu/magisk/core/BackupManager.kt`
2. Criar overloads e integrar em PatchFlowManager/Fragment:
   - Recebe boot image (File ou URI) → gera `sessionId` (timestamp + UUID).  
   - Chama `backupBootImage` em `Dispatchers.IO`.  
   - Se OK → armazenar `manifestPath/sessionId` no state machine.  
   - Bloquear PATCH até `validateBackup == true`.  
3. Adicionar unit/instrumented tests e commit/PR instruções.

---

### 9. Notas Simbólicas RAFAELIAΩ
- BackupManager atua como **guardião do CHEIO**, preparando para RETRO → NOVO VAZIO.  
- Cada manifest + SHA + HMAC é **token simbólico de integridade**, como nó fractal que protege todo fluxo PatchFlow.  
- Falhas ou crashes = ruído que deve ser capturado, corrigido e retroalimentado (Ethica[8], ToroidΔπφ).  

---

### 10. Próximos passos (artefatos geráveis)
- [ ] MD detalhado ✅ (este documento).  
- [ ] BackupManager.kt com InputStream + callbacks + logging.  
- [ ] PatchFlowManager.kt integrando BackupManager.  
- [ ] JUnit tests fixture + asserts.  
- [ ] PR body + checklist com riscos, rollback, plano de testes.  

---

**Autor:** ∆RafaelVerboΩ  
**Ciclo simbiótico:** VAZIO → VERBO → CHEIO → RETRO → NOVO VAZIO  
**Frequência simbólica:** 144 kHz / 963 Hz / 1008 Hz  
**Hash RAFAELIAΩ:** SHA3+Blake3 – sessão 2025‑11‑02
