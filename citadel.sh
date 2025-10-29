#!/bin/bash
# -x
# Franco Russomanno 336508 - Sebastian Garcia 342998

# === Archivos de trabajo ===
touch usuarios.txt
touch registro.txt
touch productos.txt 

# === Utilidades ===
trim() {
  # Imprime la cadena sin espacios al inicio/fin
  echo "$1" | sed -e 's/^[[:space:]]//;s/[[:space:]]$//'
}

user_exists() {
  # 0 si existe, 1 si no
  local u="$1"
  grep -qE "^${u}¬" usuarios.txt
}

cred_ok() {
  # 0 si usuario y password coinciden, 1 en caso contrario
  local u="$1" p="$2"
  grep -qE "^${u}¬${p}$" usuarios.txt
}

register_login() {
  # Registra la fecha/hora de login para u (único por usuario)
  local u="$1"
  grep -vE "^${u}¬" registro.txt > registro.bak && mv registro.bak registro.txt
  echo "${u}¬$(date "+%Y/%m/%d-%H:%M:%S")" >> registro.txt
}

last_login_of() {
  # Devuelve por stdout la última fecha de login para u (o vacío si no hay)
  local u="$1"
  grep -E "^${u}¬" registro.txt | tail -n 1 | awk -F"¬" '{print $2}'
}

change_password() {
  local u="$1"
  local actual nueva confirm intentos=0 ok=0

  # Verificar contraseña actual (3 intentos)
  while [ $intentos -lt 3 ]; do
    read -s -p "Ingrese la contraseña actual de ${u}: " actual; echo
    if cred_ok "$u" "$actual"; then ok=1; break; else echo "Contraseña incorrecta."; fi
    intentos=$((intentos+1))
  done
  if [ $ok -ne 1 ]; then
    echo "Demasiados intentos fallidos."
    return 1
  fi

  # Nueva contraseña (no vacía, confirmada y distinta)
  while true; do
    read -s -p "Ingrese la nueva contraseña: " nueva; echo
    if [ -z "$nueva" ]; then echo "La contraseña no puede estar en blanco."; continue; fi
    read -s -p "Confirme la nueva contraseña: " confirm; echo
    if [ "$nueva" != "$confirm" ]; then echo "Las contraseñas no coinciden."; continue; fi
    if [ "$nueva" = "$actual" ]; then echo "La nueva contraseña no puede ser igual a la anterior."; continue; fi
    break
  done

  # Actualización atómica de usuarios.txt
  awk -F"¬" -v OFS="¬" -v usr="$u" -v pass="$nueva" '{
    if ($1==usr) { $2=pass } print
  }' usuarios.txt > usuarios.bak && mv usuarios.bak usuarios.txt

  echo "Contraseña cambiada correctamente."
}
# === Parte 2: Ingresar producto (AGREGADO) ===
ingresar_producto() {
  local tipo modelo desc cantidad precio codigo

  # Tipo
  read -p "Ingrese el Tipo (nombre completo): " tipo
  tipo="$(trim "$tipo")"
  if [ -z "$tipo" ]; then echo "El Tipo no puede ser vacío."; return 1; fi

  # Código = primeras 3 letras del tipo en MAYÚSCULA
  codigo="$(echo "$tipo" | tr '[:lower:]' '[:upper:]' | sed 's/[^A-Z]//g' | cut -c1-3)"
  if [ -z "$codigo" ]; then echo "No se pudo generar el código (revise el Tipo)."; return 1; fi

  # Modelo
  read -p "Ingrese el Modelo: " modelo
  modelo="$(trim "$modelo")"
  if [ -z "$modelo" ]; then echo "El Modelo no puede ser vacío."; return 1; fi

  # Descripción
  read -p "Ingrese la Descripción del producto: " desc
  desc="$(trim "$desc")"
  if [ -z "$desc" ]; then echo "La Descripción no puede ser vacía."; return 1; fi

  # Cantidad (entero)
  read -p "Ingrese la Cantidad: " cantidad
  cantidad="$(trim "$cantidad")"
  if ! [[ "$cantidad" =~ ^[0-9]+$ ]]; then
    echo "La Cantidad debe ser un entero (>= 0)."; return 1
  fi

  # Precio unitario (entero)
  read -p "Ingrese el Precio unitario (entero): " precio
  precio="$(trim "$precio")"
  if ! [[ "$precio" =~ ^[0-9]+$ ]]; then
    echo "El Precio debe ser un entero (>= 0)."; return 1
  fi

  # Mostrar y guardar salida con el formato solicitado
  local linea="${codigo} - ${tipo} - ${modelo} - ${desc} - ${cantidad} - \$ ${precio}"
  echo "$linea"
  echo "$linea" >> productos.txt
  echo "Producto registrado en productos.txt"
}





# === Seed admin/admin si no existe ===
if ! user_exists "admin"; then
  echo "admin¬admin" >> usuarios.txt
fi

clear
echo "¡Bienvenido!"
PS3='Ingrese su opción: '
opciones=("1) Registrar usuario" "2) Login de usuario" "Salir")
select opt in "${opciones[@]}"; do
  case "$opt" in
    "1) Registrar usuario")
      read -p "Ingrese el nombre de usuario: " miusuario
      miusuario="$(trim "$miusuario")"
       # No permitir espacios en el nombre de usuario (evita romper el formato)
      if [[ "$miusuario" =~ [[:space:]] ]]; then
        echo "El nombre de usuario no puede contener espacios."
        continue
      fi
      read -s -p "Ingrese la password para el usuario $miusuario: " mipalabra; echo

      if [ -z "$miusuario" ]; then
        echo "El nombre del usuario no puede ser nulo."
      elif [ -z "$mipalabra" ]; then
        echo "La password del usuario no puede ser nula."
      elif user_exists "$miusuario"; then
        echo "El usuario $miusuario ya existe en el sistema."
      else
        echo "${miusuario}¬${mipalabra}" >> usuarios.txt
        echo "Usuario $miusuario registrado."
      fi
      ;;

    "2) Login de usuario")
      read -p "Ingrese el nombre de usuario: " miusuario
      miusuario="$(trim "$miusuario")"
      read -s -p "Ingrese la password para el usuario $miusuario: " mipalabra; echo

      if cred_ok "$miusuario" "$mipalabra"; then
        echo -e "\nBienvenido $miusuario."
        ult=$(last_login_of "$miusuario")
        if [ -z "$ult" ]; then
          echo -e "Este es su primer ingreso.\n"
        else
          echo -e "Usted ingresó por última vez el $ult\n"
        fi
        register_login "$miusuario"

        # === SUBMENÚ DE USUARIO LOGUEADO ===
        while true; do
          echo -e "2.1) Cambiar contraseña\n2.2) Logout\n2.3 Ingresar producto "
          read -p "Elija una opción (1-3): " subopt
          case "$subopt" in
            1)
              change_password "$miusuario"
              ;;
            2)
              echo "Sesión finalizada para $miusuario."
              break
              ;;
               3)
              ingresar_producto
              ;;
            *)
              echo "Opción inválida."
              ;;
          esac
        done
      else
        echo "Usuario o clave incorrectos."
      fi
      ;;

    "Salir")
      break
      ;;

    *)
      echo "No es una opción válida ($REPLY)"
      ;;
  esac

  echo -e "\nMenú Principal\n1) Registrar usuario\n2) Login de usuario\n3) Salir"
done