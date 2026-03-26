#!/usr/bin/env python3
"""
round_corners.py — Arrondit les coins d'une image ou de tout un dossier d'images.
Usage:
    python round_corners.py "chemin/vers/image.png" <rayon> [newname_without_extension]
    python round_corners.py "chemin/vers/dossier/"  <rayon> [chemin/vers/dossier_sortie]
Exemples — fichier unique :
    python round_corners.py "card.png" 40
    python round_corners.py "card.png" 60 "card_rounded"
    python round_corners.py "card.png" 10%
Exemples — dossier :
    python round_corners.py "images/" 40
    python round_corners.py "images/" 10% "images_rounded/"
Arguments:
    source      : Fichier image (PNG, JPG, JPEG, WEBP…) ou dossier contenant des images
    rayon       : Rayon des coins arrondis, en pixels (ex: 40) ou en % de la plus petite dimension (ex: 10%)
    dest        : (Optionnel) Nom de sortie sans extension (mode fichier) ou dossier de sortie (mode dossier)
                  En mode dossier, les fichiers traités sont sauvegardés dans ce sous-dossier.
                  Si omis, chaque fichier est sauvegardé à côté de l'original avec le suffixe _r<rayon>.
"""
import sys
from pathlib import Path
import numpy as np
from PIL import Image, ImageDraw


def parse_radius(radius_str: str, img_width: int, img_height: int) -> int:
    """Parse le rayon en pixels ou en pourcentage de la plus petite dimension."""
    radius_str = radius_str.strip()
    if radius_str.endswith("%"):
        try:
            pct = float(radius_str[:-1])
        except ValueError:
            raise ValueError(f"Pourcentage invalide : '{radius_str}'. Exemple valide : '10%'")
        if not 0 < pct <= 50:
            raise ValueError(f"Le pourcentage ({pct}%) doit être entre 0 et 50.")
        radius = int(min(img_width, img_height) * pct / 100)
    else:
        try:
            radius = int(radius_str)
        except ValueError:
            raise ValueError(f"Rayon invalide : '{radius_str}'. Entrez un entier (ex: 40) ou un pourcentage (ex: 10%).")
        if radius <= 0:
            raise ValueError(f"Le rayon ({radius}) doit être un entier positif.")

    max_radius = min(img_width, img_height) // 2
    if radius > max_radius:
        raise ValueError(
            f"Le rayon ({radius}px) est trop grand pour cette image ({img_width}×{img_height}px). "
            f"Maximum autorisé : {max_radius}px."
        )
    return radius


def round_corners(image_path: str, radius: int, output_name: str | None = None) -> None:
    """
    Applique un masque à coins arrondis sur l'image et sauvegarde le résultat en PNG.
    Le canal alpha existant est préservé : les coins deviennent transparents.
    """
    src = Path(image_path)
    if not src.exists():
        raise FileNotFoundError(f"Fichier introuvable : {image_path}")

    img = Image.open(src).convert("RGBA")
    width, height = img.size

    # --- Construction du masque de coins arrondis ---
    # On dessine un rectangle à coins arrondis en blanc sur fond noir.
    mask = Image.new("L", (width, height), 0)
    draw = ImageDraw.Draw(mask)

    # Dessine le rectangle arrondi (blanc = opaque)
    draw.rounded_rectangle([(0, 0), (width - 1, height - 1)], radius=radius, fill=255)

    # --- Application du masque ---
    # Si l'image possède déjà un canal alpha, on combine les deux masques
    # (intersection : un pixel n'est visible que s'il l'est dans les deux).
    r, g, b, a = img.split()
    mask_arr    = np.array(mask,      dtype=np.uint16)
    alpha_arr   = np.array(a,         dtype=np.uint16)
    combined    = np.clip((mask_arr * alpha_arr) // 255, 0, 255).astype(np.uint8)
    new_alpha   = Image.fromarray(combined, mode="L")

    result = Image.merge("RGBA", (r, g, b, new_alpha))

    # --- Chemin de sortie ---
    if output_name:
        p = Path(output_name)
        out_path = p if p.suffix.lower() == ".png" else p.with_suffix(".png")
    else:
        out_path = src.with_stem(src.stem + f"_r{radius}")

    result.save(out_path, format="PNG")
    print(f"✔ Image sauvegardée : {out_path}")
    print(f"  Dimensions : {width}×{height}px  |  Rayon des coins : {radius}px")


SUPPORTED_EXTENSIONS = {".png", ".jpg", ".jpeg", ".webp", ".bmp", ".tiff", ".tif"}


def process_file(src: Path, radius_str: str, out_path: Path | None = None) -> bool:
    """
    Traite un fichier image unique.
    Retourne True si le traitement a réussi, False sinon.
    """
    try:
        with Image.open(src) as tmp:
            w, h = tmp.size
        radius = parse_radius(radius_str, w, h)
        print(f"  → {src.name}  ({w}×{h}px)  rayon={radius}px")
        round_corners(str(src), radius, str(out_path.with_suffix("")) if out_path else None)
        return True
    except (ValueError, FileNotFoundError, Exception) as e:
        print(f"  ✖ {src.name} — ignoré : {e}", file=sys.stderr)
        return False


def process_folder(folder: Path, radius_str: str, out_folder: Path | None = None) -> None:
    """
    Applique le traitement sur toutes les images d'un dossier.
    Si out_folder est fourni, les résultats y sont écrits (dossier créé si nécessaire).
    Sinon, chaque image est sauvegardée à côté de l'originale avec le suffixe _r<rayon>.
    """
    images = sorted(
        f for f in folder.rglob("*")
        if f.is_file() and f.suffix.lower() in SUPPORTED_EXTENSIONS
    )

    if not images:
        print(f"✖ Aucune image trouvée dans : {folder}", file=sys.stderr)
        sys.exit(1)

    if out_folder:
        out_folder.mkdir(parents=True, exist_ok=True)
        print(f"→ Dossier source  : {folder}  ({len(images)} image(s))")
        print(f"→ Dossier sortie  : {out_folder}  (structure des sous-dossiers reproduite)")
    else:
        print(f"→ Dossier source  : {folder}  ({len(images)} image(s))")
        print(f"→ Dossier sortie  : (même dossier que chaque source, suffixe _r<rayon>)")

    print()
    ok = ko = 0
    for img_path in images:
        if out_folder:
            # Reproduit l'arborescence relative dans le dossier de sortie
            relative = img_path.relative_to(folder)
            out_path = (out_folder / relative).with_suffix(".png")
            out_path.parent.mkdir(parents=True, exist_ok=True)
        else:
            out_path = None
        if process_file(img_path, radius_str, out_path):
            ok += 1
        else:
            ko += 1

    print()
    print(f"✔ Terminé — {ok} image(s) traitée(s)", end="")
    print(f", {ko} ignorée(s)." if ko else ".")


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)

    source     = Path(sys.argv[1])
    radius_str = sys.argv[2]
    dest       = sys.argv[3] if len(sys.argv) > 3 else None

    try:
        if not source.exists():
            raise FileNotFoundError(f"Chemin introuvable : {source}")

        # --- Mode dossier ---
        if source.is_dir():
            out_folder = Path(dest) if dest else None
            process_folder(source, radius_str, out_folder)

        # --- Mode fichier unique ---
        else:
            with Image.open(source) as tmp:
                w, h = tmp.size
            radius = parse_radius(radius_str, w, h)
            out_path = Path(dest + ".png") if dest else None
            print(f"→ Image source  : {source}  ({w}×{h}px)")
            print(f"→ Rayon cible   : {radius}px")
            round_corners(str(source), radius, dest)

    except (ValueError, FileNotFoundError) as e:
        print(f"✖ Erreur : {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
