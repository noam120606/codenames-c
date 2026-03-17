#!/usr/bin/env python3
"""
change_color.py — Recolore un bouton PNG en conservant le canal alpha et les détails.

Usage:
    python change_color.py "chemin/vers/image.png" "{R,G,B}" "newname_without_extension"

Exemple:
    python change_color.py "button.png" "{255,50,50}" "new_button_red"
    python change_color.py "button.png" "{50,120,255}" "new_button_blue"
"""

import sys
import re
from pathlib import Path
import numpy as np
from PIL import Image


def parse_color(color_str: str) -> tuple[int, int, int]:
    """Parse une couleur au format {R,G,B} ou R,G,B."""
    cleaned = re.sub(r"[{}\s]", "", color_str)
    parts = cleaned.split(",")
    if len(parts) != 3:
        raise ValueError(f"Format de couleur invalide : '{color_str}'. Attendu : {{R,G,B}}")
    r, g, b = [int(p) for p in parts]
    for val, name in zip([r, g, b], ["R", "G", "B"]):
        if not 0 <= val <= 255:
            raise ValueError(f"La composante {name}={val} doit être entre 0 et 255.")
    return r, g, b


def recolor_image(image_path: str, target_rgb: tuple[int, int, int]) -> None:
    """
    Recolore l'image en appliquant la teinte cible tout en préservant
    la luminosité relative (les reflets, ombres, textures) et le canal alpha.
    """
    src = Path(image_path)
    if not src.exists():
        raise FileNotFoundError(f"Fichier introuvable : {image_path}")

    img = Image.open(src).convert("RGBA")
    arr = np.array(img, dtype=np.float32)

    r, g, b, a = arr[..., 0], arr[..., 1], arr[..., 2], arr[..., 3]

    # Luminosité perceptuelle de chaque pixel (0.0 → 1.0)
    luminance = (0.299 * r + 0.587 * g + 0.114 * b) / 255.0

    # Applique la couleur cible modulée par la luminance
    tr, tg, tb = target_rgb
    new_r = np.clip(luminance * tr, 0, 255)
    new_g = np.clip(luminance * tg, 0, 255)
    new_b = np.clip(luminance * tb, 0, 255)

    # Reconstruction du tableau RGBA
    result = np.stack([new_r, new_g, new_b, a], axis=-1).astype(np.uint8)

    out_img = Image.fromarray(result, mode="RGBA")

    # Sauvegarde dans le même dossier, avec suffixe de couleur si aucun nom de sortie n'est donné
    if len(sys.argv) > 3:
        out_path = src.with_name(sys.argv[3] + ".png")
    else:
        suffix = f"_{tr}-{tg}-{tb}"
        out_path = src.with_stem(src.stem + suffix)
    out_img.save(out_path, format="PNG")

    print(f"✔ Image sauvegardée : {out_path}")


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)

    image_path = sys.argv[1]
    color_str  = sys.argv[2]

    try:
        target_rgb = parse_color(color_str)
        print(f"→ Couleur cible : RGB{target_rgb}")
        recolor_image(image_path, target_rgb)
    except (ValueError, FileNotFoundError) as e:
        print(f"✖ Erreur : {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
